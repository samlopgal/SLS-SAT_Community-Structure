// ExperimentRunner.cpp

#include "ExperimentRunner.h"

#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <omp.h>
#include <sstream>
#include <tuple>
#include <vector>

#include "CSVLogger.h"
#include "FileToVector.h"
#include "GenericStatsCollector.h"
#include "InstanceScanner.h"
#include "PrecomputedCommunityIO.h"
#include "SolverFactory.h"
#include "SolutionVerifier.h"

namespace fs = std::filesystem;

namespace {

std::string formatETA(double seconds)
{
    if (seconds < 0.0 || !std::isfinite(seconds)) {
        return "--:--:--";
    }

    int total = static_cast<int>(seconds + 0.5);
    int h = total / 3600;
    int m = (total % 3600) / 60;
    int s = total % 60;

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << h << ":"
        << std::setw(2) << m << ":"
        << std::setw(2) << s;

    return oss.str();
}

using FullGroupKey = std::tuple<std::string, int, int, int>; // type, n, q, ratio
using ScopeKey = std::tuple<std::string, int, int>;           // type, n, q

ScopeKey scopeFromInstance(const InstanceInfo& instance)
{
    return std::make_tuple(
        instance.type,
        instance.n,
        instance.q
    );
}

FullGroupKey fullKeyFromInstance(const InstanceInfo& instance)
{
    return std::make_tuple(
        instance.type,
        instance.n,
        instance.q,
        instance.ratio
    );
}

} // namespace

ExperimentRunner::ExperimentRunner(const ExperimentConfig& config)
    : config(config)
{
}

void ExperimentRunner::run()
{
    std::cout << std::unitbuf;

    std::vector<InstanceInfo> instances =
        InstanceScanner::scan(config.instances);

    if (instances.empty()) {
        std::cout << "No se encontraron instancias.\n";
        return;
    }

    const std::size_t total_instances = instances.size();

    std::cout << "Instancias detectadas: "
              << total_instances << "\n";

    const int auto_stop_patience =
        config.auto_stopping_patience_ratios;

    if (auto_stop_patience > 0) {
        std::cout
            << "Auto-stopping activado: un solver se detiene "
            << "tras " << auto_stop_patience
            << " ratios consecutivos sin resolver ninguna instancia "
            << "dentro de cada grupo (type,n,q).\n";
    }
    else {
        std::cout << "Auto-stopping desactivado.\n";
    }

    fs::create_directories(config.logging.output_dir);

    CSVLogger logger(
        config.logging.output_dir,
        config.logging.instance_results_file,
        config.logging.summary_results_file
    );

    int num_threads = config.execution.threads;
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }

    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);

    std::cout << "Usando "
              << num_threads
              << " hilos.\n";

    // Agrupar por (type,n,q,ratio). El std::map mantiene orden creciente de ratio
    // dentro de cada scope (type,n,q), que es necesario para auto-stopping.
    std::map<FullGroupKey, std::vector<int>> groups;
    for (int i = 0; i < static_cast<int>(instances.size()); ++i) {
        groups[fullKeyFromInstance(instances[i])].push_back(i);
    }

    Statistics global_stats;

    std::mutex progress_mutex;
    std::size_t completed_instances = 0;
    constexpr std::size_t PROGRESS_UPDATE_EVERY = 2;

    auto global_start = std::chrono::steady_clock::now();

    auto update_progress = [&](const InstanceInfo& instance) {
        std::lock_guard<std::mutex> lock(progress_mutex);

        ++completed_instances;

        const bool should_print =
            (completed_instances == 1) ||
            (completed_instances % PROGRESS_UPDATE_EVERY == 0) ||
            (completed_instances == total_instances);

        if (should_print) {
            double percent =
                100.0 * static_cast<double>(completed_instances) /
                static_cast<double>(total_instances);

            auto now = std::chrono::steady_clock::now();

            double elapsed =
                std::chrono::duration<double>(now - global_start).count();

            double avg_time =
                elapsed / static_cast<double>(completed_instances);

            double remaining =
                avg_time * static_cast<double>(
                    total_instances - completed_instances
                );

            std::cout
                << "["
                << std::fixed
                << std::setw(6)
                << std::setprecision(2)
                << percent
                << "%] "
                << completed_instances
                << "/"
                << total_instances
                << " | ETA "
                << formatETA(remaining)
                << " | "
                << instance.filename
                << "\n";
        }
    };

    std::vector<bool> solver_active(config.solvers.size(), false);
    std::vector<int> solver_fail_streak(config.solvers.size(), 0);
    ScopeKey current_scope;
    bool have_scope = false;

    for (const auto& group_entry : groups) {
        const std::vector<int>& group_indices = group_entry.second;
        if (group_indices.empty()) {
            continue;
        }

        const InstanceInfo& first_instance =
            instances[group_indices.front()];

        ScopeKey scope = scopeFromInstance(first_instance);

        if (!have_scope || scope != current_scope) {
            current_scope = scope;
            have_scope = true;

            for (std::size_t i = 0; i < config.solvers.size(); ++i) {
                solver_active[i] = config.solvers[i].enabled;
                solver_fail_streak[i] = 0;
            }

            if (auto_stop_patience > 0) {
                std::cout
                    << "Nuevo grupo auto-stopping: type="
                    << first_instance.type
                    << ", n=" << first_instance.n
                    << ", q=" << first_instance.q
                    << "\n";
            }
        }

        bool any_active_solver = false;
        for (std::size_t i = 0; i < config.solvers.size(); ++i) {
            if (config.solvers[i].enabled && solver_active[i]) {
                any_active_solver = true;
                break;
            }
        }

        if (!any_active_solver) {
            std::cout
                << "Todos los solvers detenidos para type="
                << first_instance.type
                << ", n=" << first_instance.n
                << ", q=" << first_instance.q
                << ". Se omite ratio="
                << first_instance.ratio
                << ".\n";

            for (int instance_idx : group_indices) {
                update_progress(instances[instance_idx]);
            }
            continue;
        }

        std::vector<int> ratio_solved_counts(config.solvers.size(), 0);

#pragma omp parallel
        {
            Statistics local_stats;
            std::vector<InstanceResult> local_results;
            local_results.reserve(512);
            std::vector<int> local_solved_counts(config.solvers.size(), 0);

#pragma omp for schedule(guided)
            for (int pos = 0;
                 pos < static_cast<int>(group_indices.size());
                 ++pos)
            {
                const int instance_idx = group_indices[pos];
                const InstanceInfo& instance = instances[instance_idx];

                int num_variables = 0;

                std::vector<std::vector<int>> formula =
                    FileToVector::readCNF(
                        instance.filepath,
                        num_variables
                    );

                if (formula.empty()) {
#pragma omp critical(progress_error)
                    {
                        std::cerr
                            << "Error leyendo "
                            << instance.filepath
                            << "\n";
                    }

                    update_progress(instance);
                    continue;
                }

                std::unique_ptr<PrecomputedCommunityData>
                    precomputed_ptr;

                PrecomputedCommunityData* precomputed = nullptr;

                try {
                    fs::path pcd_path(config.precomputed.root_path);

                    if (instance.type == "random") {
                        pcd_path /= "random";
                        pcd_path /= "n" + std::to_string(instance.n);
                        pcd_path /= "r_" + std::to_string(instance.ratio);
                    }
                    else if (instance.type == "community" ||
                             instance.type == "community_instances")
                    {
                        pcd_path /= "community_instances";
                        pcd_path /= "q_" + std::to_string(instance.q);
                        pcd_path /= "n" + std::to_string(instance.n);
                        pcd_path /= "r_" + std::to_string(instance.ratio);
                    }

                    std::string stem =
                        fs::path(instance.filename).stem().string();

                    pcd_path /= stem + ".pcd";

                    if (fs::exists(pcd_path)) {
                        precomputed_ptr =
                            std::make_unique<PrecomputedCommunityData>(
                                PrecomputedCommunityIO::load(
                                    pcd_path.string()
                                )
                            );

                        precomputed = precomputed_ptr.get();
                    }
                }
                catch (...) {
                    precomputed = nullptr;
                }

                for (std::size_t solver_index = 0;
                     solver_index < config.solvers.size();
                     ++solver_index)
                {
                    const auto& solver_cfg = config.solvers[solver_index];

                    if (!solver_cfg.enabled) {
                        continue;
                    }

                    if (!solver_active[solver_index]) {
                        continue;
                    }

                    std::vector<int> seeds;
                    if (!solver_cfg.params.seeds.empty()) {
                        seeds = solver_cfg.params.seeds;
                    }
                    else {
                        seeds.push_back(solver_cfg.params.seed);
                    }

                    for (int current_seed : seeds) {
                        SolverParameters params;
                        params.seed   = current_seed;
                        params.alpha  = solver_cfg.params.alpha;
                        params.lambda = solver_cfg.params.lambda;
                        params.maxinc = solver_cfg.params.maxinc;
                        params.p_soft = solver_cfg.params.p_soft;

                        int effective_max_flips =
                            solver_cfg.params.max_flips;

                        if (solver_cfg.params.max_flips_per_variable > 0.0) {
                            effective_max_flips =
                                static_cast<int>(
                                    solver_cfg.params.max_flips_per_variable *
                                    static_cast<double>(num_variables)
                                );
                        }

                        std::unique_ptr<ISolver> solver;

                        try {
                            solver = SolverFactory::create(
                                solver_cfg.name,
                                formula,
                                num_variables,
                                params,
                                precomputed
                            );
                        }
                        catch (const std::exception& e) {
#pragma omp critical(progress_error)
                            {
                                std::cerr
                                    << "Error creando solver "
                                    << solver_cfg.name
                                    << " (seed=" << current_seed << "): "
                                    << e.what()
                                    << "\n";
                            }
                            continue;
                        }

                        if (!solver) {
                            continue;
                        }

                        GenericStatsCollector stats_collector(
                            num_variables,
                            static_cast<int>(formula.size())
                        );

                        auto start =
                            std::chrono::high_resolution_clock::now();

                        bool reported_solved = solver->solve(
                            effective_max_flips,
                            solver_cfg.params.max_tries,
                            solver_cfg.params.p,
                            &stats_collector
                        );

                        auto end =
                            std::chrono::high_resolution_clock::now();

                        double runtime_ms =
                            std::chrono::duration<double, std::milli>(
                                end - start
                            ).count();

                        bool verified_solved = false;
                        if (reported_solved) {
                            verified_solved = SolutionVerifier::verify(
                                formula,
                                num_variables,
                                solver->getAssignment()
                            );
                        }

                        bool invalid_solution =
                            reported_solved && !verified_solved;

                        GenericSolverStats run_stats =
                            stats_collector.stats();
                        run_stats.time_ms = runtime_ms;
                        run_stats.reported_solved = reported_solved;
                        run_stats.verified_solved = verified_solved;
                        run_stats.invalid_solution = invalid_solution;

                        InstanceResult result;
                        result.solver = solver_cfg.name;
                        result.instance_type = instance.type;
                        result.q = instance.q;
                        result.n = instance.n;
                        result.ratio = instance.ratio;
                        result.filename = instance.filename;
                        result.seed = current_seed;
                        result.p = solver_cfg.params.p;
                        result.maxinc = solver_cfg.params.maxinc;
                        result.alpha = solver_cfg.params.alpha;
                        result.lambda = solver_cfg.params.lambda;
                        result.solved = verified_solved;
                        result.reported_solved = reported_solved;
                        result.verified_solved = verified_solved;
                        result.invalid_solution = invalid_solution;
                        result.runtime_ms = runtime_ms;
                        result.stats = run_stats;

                        local_results.push_back(result);
                        local_stats.add(result);

                        if (verified_solved) {
                            ++local_solved_counts[solver_index];
                        }
                    }
                }

                update_progress(instance);
            }

#pragma omp critical(results_merge)
            {
                for (const auto& r : local_results) {
                    logger.logInstanceResult(r);
                }

                global_stats.merge(local_stats);

                for (std::size_t i = 0;
                     i < ratio_solved_counts.size();
                     ++i)
                {
                    ratio_solved_counts[i] += local_solved_counts[i];
                }
            }
        }

        if (auto_stop_patience > 0) {
            for (std::size_t solver_index = 0;
                 solver_index < config.solvers.size();
                 ++solver_index)
            {
                if (!config.solvers[solver_index].enabled ||
                    !solver_active[solver_index])
                {
                    continue;
                }

                const int solved_count =
                    ratio_solved_counts[solver_index];

                if (solved_count > 0) {
                    solver_fail_streak[solver_index] = 0;
                }
                else {
                    ++solver_fail_streak[solver_index];
                }

                if (solver_fail_streak[solver_index] >=
                    auto_stop_patience)
                {
                    solver_active[solver_index] = false;

                    AutoStoppingEvent event;
                    event.instance_type = first_instance.type;
                    event.q = first_instance.q;
                    event.n = first_instance.n;
                    event.ratio = first_instance.ratio;
                    event.solver = config.solvers[solver_index].name;
                    event.solved_count = solved_count;
                    event.fail_streak = solver_fail_streak[solver_index];
                    event.patience_ratios = auto_stop_patience;
                    event.event = "stopped";

                    logger.logAutoStoppingEvent(event);

                    std::cout
                        << "Auto-stopping: solver "
                        << config.solvers[solver_index].name
                        << " detenido para type="
                        << first_instance.type
                        << ", n=" << first_instance.n
                        << ", q=" << first_instance.q
                        << " después de ratio="
                        << first_instance.ratio
                        << " ("
                        << solver_fail_streak[solver_index]
                        << " ratios consecutivos sin resolver).\n";
                }
            }
        }
    }

    std::vector<SummaryResult> summaries =
        global_stats.computeSummary();

    for (const auto& summary : summaries) {
        logger.logSummaryResult(summary);
    }

    std::cout << "Experimento finalizado.\n";
    std::cout << "Resultados guardados en: "
              << config.logging.output_dir
              << "\n";
}
