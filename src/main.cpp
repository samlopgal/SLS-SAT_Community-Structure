#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <atomic>
#include <iomanip>
#include <set>

#include <omp.h>

#include "ConfigLoader.h"
#include "ExperimentRunner.h"

#include "InstanceScanner.h"
#include "FileToVector.h"
#include "InstanceProcessing.h"
#include "PrecomputedCommunityBuilder.h"
#include "PrecomputedCommunityIO.h"
#include "PrecomputeUtils.h"

namespace fs = std::filesystem;

static bool isBlank(const std::string& s)
{
    return std::all_of(
        s.begin(),
        s.end(),
        [](unsigned char c) {
            return std::isspace(c);
        }
    );
}

static bool precomputedExists(const fs::path& path)
{
    std::error_code ec;

    if (!fs::exists(path, ec) || ec) {
        return false;
    }

    if (!fs::is_regular_file(path, ec) || ec) {
        return false;
    }

    const auto size = fs::file_size(path, ec);

    if (ec) {
        return false;
    }

    return size > 0;
}

static void ensurePrecomputedCommunities(const ExperimentConfig& config)
{
    std::cout
        << "\nChecking precomputed communities..."
        << std::endl;

    std::vector<InstanceInfo> instances =
        InstanceScanner::scan(config.instances);

    if (instances.empty()) {
        std::cout
            << "No instances found for precomputation."
            << std::endl;
        return;
    }

    struct PrecomputeTask {
        InstanceInfo instance;
        fs::path output_path;
    };

    std::vector<PrecomputeTask> missing_tasks;
    std::set<std::string> scheduled_paths;

    for (const auto& instance : instances) {
        fs::path output_path =
            getPrecomputedPath(
                instance,
                config.precomputed.root_path
            );

        if (precomputedExists(output_path)) {
            continue;
        }

        std::string normalized_path =
            output_path.lexically_normal().string();

        if (scheduled_paths.insert(normalized_path).second) {
            missing_tasks.push_back(
                PrecomputeTask{
                    instance,
                    output_path
                }
            );
        }
    }

    if (missing_tasks.empty()) {
        std::cout
            << "All precomputed community files already exist."
            << std::endl;
        return;
    }

    const int total_missing =
        static_cast<int>(missing_tasks.size());

    std::cout
        << "Missing precomputed files: "
        << total_missing
        << ". Generating now..."
        << std::endl;

    omp_set_num_threads(8);

    std::cout
        << "Using "
        << omp_get_max_threads()
        << " OpenMP threads."
        << std::endl;

    std::atomic<int> completed{0};
    std::atomic<int> processed{0};
    std::atomic<int> skipped{0};

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < total_missing; ++i) {
        const auto& task = missing_tasks[i];
        const auto& instance = task.instance;
        const fs::path& output_path = task.output_path;

        try {
            if (precomputedExists(output_path)) {
                skipped.fetch_add(
                    1,
                    std::memory_order_relaxed
                );
            }
            else {
                int num_variables = 0;

                std::vector<std::vector<int>> formula =
                    FileToVector::readCNF(
                        instance.filepath,
                        num_variables
                    );

                if (!formula.empty()) {
                    auto graph =
                        InstanceProcessing::buildWeightedVIG(
                            formula,
                            num_variables,
                            true
                        );

                    std::vector<int> var_to_community =
                        InstanceProcessing::runLouvain(
                            graph
                        );

                    PrecomputedCommunityData data =
                        PrecomputedCommunityBuilder::build(
                            formula,
                            var_to_community
                        );

                    fs::create_directories(
                        output_path.parent_path()
                    );

                    PrecomputedCommunityIO::save(
                        output_path.string(),
                        data
                    );

                    processed.fetch_add(
                        1,
                        std::memory_order_relaxed
                    );
                }
                else {
#pragma omp critical
                    {
                        std::cerr
                            << "\nWarning: empty formula in "
                            << instance.filepath
                            << std::endl;
                    }
                }
            }
        }
        catch (const std::exception& e) {
#pragma omp critical
            {
                std::cerr
                    << "\nError precomputing "
                    << instance.filepath
                    << ": "
                    << e.what()
                    << std::endl;
            }
        }

        int done =
            completed.fetch_add(
                1,
                std::memory_order_relaxed
            ) + 1;

        double percent =
            100.0 * done / total_missing;

#pragma omp critical
        {
            std::cout
                << "\rPrecompute progress: "
                << done
                << "/"
                << total_missing
                << " ("
                << std::fixed
                << std::setprecision(1)
                << percent
                << "%)"
                << std::flush;
        }
    }

    std::cout << std::endl;

    std::cout
        << "Precomputation finished. "
        << processed.load()
        << " files generated, "
        << skipped.load()
        << " skipped because they already existed."
        << std::endl;
}

int main(int argc, char* argv[])
{
    try {
        std::cout
            << "Current working directory: "
            << fs::current_path()
            << std::endl;

        // ----------------------------------------------------
        // Ficheros de configuración
        // ----------------------------------------------------
        std::vector<std::string> config_files;

        if (argc > 1) {
            for (int i = 1; i < argc; ++i) {
                config_files.push_back(argv[i]);
            }
        }
        else {
            config_files = {
                "src/prueba.json"
            };
        }

        // ----------------------------------------------------
        // Ejecutar experimentos en secuencia
        // ----------------------------------------------------
        std::size_t executed = 0;

        for (std::size_t i = 0; i < config_files.size(); ++i) {
            const std::string& config_file = config_files[i];

            if (isBlank(config_file)) {
                std::cout
                    << "\nSkipping empty experiment slot "
                    << (i + 1)
                    << "/"
                    << config_files.size()
                    << "."
                    << std::endl;
                continue;
            }

            ++executed;

            std::cout
                << "\n====================================================\n"
                << "Experiment slot "
                << (i + 1)
                << "/"
                << config_files.size()
                << "\n"
                << "Loading configuration: "
                << config_file
                << "\n"
                << "===================================================="
                << std::endl;

            // ------------------------------------------------
            // Cargar configuración
            // ------------------------------------------------
            ExperimentConfig config =
                ConfigLoader::load(config_file);

            // ------------------------------------------------
            // Primera fase: asegurar precomputed
            // ------------------------------------------------
            ensurePrecomputedCommunities(config);

            // ------------------------------------------------
            // Segunda fase: ejecutar experimento
            // ------------------------------------------------
            ExperimentRunner runner(config);
            runner.run();

            std::cout
                << "\nFinished experiment slot "
                << (i + 1)
                << "/"
                << config_files.size()
                << ": "
                << config_file
                << std::endl;
        }

        if (executed == 0) {
            std::cout
                << "\nNo experiments were executed: all config entries were empty."
                << std::endl;
        }
        else {
            std::cout
                << "\nAll non-empty experiments finished successfully."
                << std::endl;
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr
            << "Fatal error: "
            << e.what()
            << std::endl;

        return 1;
    }
}
