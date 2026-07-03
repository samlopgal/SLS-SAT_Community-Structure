#include "CSVLogger.h"

#include <iomanip>
#include <stdexcept>

namespace fs = std::filesystem;

CSVLogger::CSVLogger(
    const std::string& output_dir,
    const std::string& instance_results_file,
    const std::string& summary_results_file)
{
    fs::create_directories(output_dir);

    fs::path instance_path =
        fs::path(output_dir) / instance_results_file;

    fs::path summary_path =
        fs::path(output_dir) / summary_results_file;

    fs::path auto_stopping_path =
        fs::path(output_dir) / "auto_stopping_events.csv";

    instance_stream.open(instance_path);
    summary_stream.open(summary_path);
    auto_stopping_stream.open(auto_stopping_path);

    if (!instance_stream.is_open()) {
        throw std::runtime_error(
            "Cannot open instance results file: "
            + instance_path.string()
        );
    }

    if (!summary_stream.is_open()) {
        throw std::runtime_error(
            "Cannot open summary results file: "
            + summary_path.string()
        );
    }

    if (!auto_stopping_stream.is_open()) {
        throw std::runtime_error(
            "Cannot open auto-stopping events file: "
            + auto_stopping_path.string()
        );
    }

    writeHeaders();
}

CSVLogger::~CSVLogger()
{
    if (instance_stream.is_open())
        instance_stream.close();

    if (summary_stream.is_open())
        summary_stream.close();

    if (auto_stopping_stream.is_open())
        auto_stopping_stream.close();
}

void CSVLogger::writeHeaders()
{
    instance_stream
        << "solver,"
        << "instance_type,"
        << "q,"
        << "n,"
        << "ratio,"
        << "filename,"
		<< "seed,"
		<< "p,"
		<< "maxinc,"
		<< "alpha,"
		<< "lambda,"
        << "bridge_clause_probability,"
        << "reported_solved,"
        << "verified_solved,"
        << "invalid_solution,"
        << "runtime_ms,"
        << "flips_used,"
        << "tries_used,"
        << "initial_unsat,"
        << "final_unsat,"
        << "best_unsat,"
        << "flips_to_first_improvement,"
        << "flips_to_best,"
        << "improving_steps,"
        << "neutral_steps,"
        << "worsening_steps,"
        << "random_steps,"
        << "random_improving_steps,"
        << "random_neutral_steps,"
        << "random_worsening_steps,"
        << "heuristic_steps,"
        << "heuristic_improving_steps,"
        << "heuristic_neutral_steps,"
        << "heuristic_worsening_steps,"
        << "unknown_steps,"
        << "unknown_improving_steps,"
        << "unknown_neutral_steps,"
        << "unknown_worsening_steps,"
        << "best_improvement_events,"
        << "largest_single_improvement,"
        << "largest_single_worsening,"
        << "avg_delta_unsat,"
        << "avg_abs_delta_unsat,"
        << "longest_stagnation,"
        << "total_stagnation_steps,"
        << "stagnation_episodes,"
        << "avg_stagnation_length,"
        << "steps_since_last_best,"
        << "distinct_vars_flipped,"
        << "immediate_backflips,"
        << "flip_entropy,"
        << "normalized_flip_entropy,"
        << "most_flipped_var_ratio,"
        << "avg_revisit_distance,"
        << "distinct_unsat_clauses_seen,"
        << "unsat_clause_entropy,"
        << "normalized_unsat_clause_entropy,"
        << "most_frequent_unsat_clause_ratio,"
        << "avg_unsat_clause_lifetime,"
        << "max_unsat_clause_lifetime,"
        << "avg_hamming_from_initial,"
        << "max_hamming_from_initial,"
        << "area_under_unsat_curve,"
        << "normalized_auc,"
        << "early_progress_rate,"
        << "late_progress_rate,"
        << "time_to_50pct_improvement,"
        << "time_to_90pct_improvement,"
        << "restarts,"
        << "successful_try,"
        << "avg_best_unsat_per_try,"
        << "std_best_unsat_per_try,"
        << "paws_penalty_events,"
        << "paws_weight_increments,"
        << "paws_smoothing_events,"
        << "paws_weight_decrements,"
        << "paws_final_weighted_clauses,"
        << "paws_max_weight_seen,"
        << "paws_final_max_weight,"
        << "paws_final_mean_weight,"
        << "paws_avg_weighted_clauses,"
        << "paws_avg_max_weight\n";

    summary_stream
        << "solver,"
        << "instance_type,"
        << "q,"
        << "n,"
        << "ratio,"
		<< "seed,"
		<< "p,"
		<< "maxinc,"
		<< "alpha,"
		<< "lambda,"
        << "bridge_clause_probability,"
        << "total_instances,"
        << "solved_instances,"
        << "reported_solved_instances,"
        << "invalid_solution_instances,"
        << "success_rate,"
        << "reported_success_rate,"
        << "invalid_solution_rate,"
        << "mean_runtime_ms,"
        << "mean_flips_used,"
        << "mean_best_unsat,"
        << "mean_final_unsat,"
        << "mean_normalized_auc,"
        << "mean_longest_stagnation,"
        << "mean_flip_entropy,"
        << "mean_random_steps,"
        << "mean_random_improving_steps,"
        << "mean_random_neutral_steps,"
        << "mean_random_worsening_steps,"
        << "mean_heuristic_steps,"
        << "mean_heuristic_improving_steps,"
        << "mean_heuristic_neutral_steps,"
        << "mean_heuristic_worsening_steps,"
        << "mean_unknown_steps,"
        << "mean_unknown_improving_steps,"
        << "mean_unknown_neutral_steps,"
        << "mean_unknown_worsening_steps,"
        << "mean_paws_penalty_events,"
        << "mean_paws_weight_increments,"
        << "mean_paws_smoothing_events,"
        << "mean_paws_weight_decrements,"
        << "mean_paws_final_weighted_clauses,"
        << "mean_paws_max_weight_seen,"
        << "mean_paws_final_max_weight,"
        << "mean_paws_final_mean_weight,"
        << "mean_paws_avg_weighted_clauses,"
        << "mean_paws_avg_max_weight\n";

    auto_stopping_stream
        << "event,"
        << "instance_type,"
        << "q,"
        << "n,"
        << "ratio,"
        << "solver,"
        << "solved_count,"
        << "fail_streak,"
        << "patience_ratios\n";
}

void CSVLogger::logInstanceResult(
    const InstanceResult& result)
{
    const auto& s = result.stats;

    instance_stream
        << result.solver << ","
        << result.instance_type << ","
        << result.q << ","
        << result.n << ","
        << result.ratio << ","
        << result.filename << ","
		<< result.seed << ","
		<< std::fixed << std::setprecision(6)
		<< result.p << ","
		<< result.maxinc << ","
		<< result.alpha << ","
		<< result.lambda << ","
        << result.bridge_clause_probability << ","
        << (result.reported_solved ? 1 : 0) << ","
        << (result.verified_solved ? 1 : 0) << ","
        << (result.invalid_solution ? 1 : 0) << ","
        << result.runtime_ms << ","
        << s.flips_used << ","
        << s.tries_used << ","
        << s.initial_unsat << ","
        << s.final_unsat << ","
        << s.best_unsat << ","
        << s.flips_to_first_improvement << ","
        << s.flips_to_best << ","
        << s.improving_steps << ","
        << s.neutral_steps << ","
        << s.worsening_steps << ","
        << s.random_steps << ","
        << s.random_improving_steps << ","
        << s.random_neutral_steps << ","
        << s.random_worsening_steps << ","
        << s.heuristic_steps << ","
        << s.heuristic_improving_steps << ","
        << s.heuristic_neutral_steps << ","
        << s.heuristic_worsening_steps << ","
        << s.unknown_steps << ","
        << s.unknown_improving_steps << ","
        << s.unknown_neutral_steps << ","
        << s.unknown_worsening_steps << ","
        << s.best_improvement_events << ","
        << s.largest_single_improvement << ","
        << s.largest_single_worsening << ","
        << s.avg_delta_unsat << ","
        << s.avg_abs_delta_unsat << ","
        << s.longest_stagnation << ","
        << s.total_stagnation_steps << ","
        << s.stagnation_episodes << ","
        << s.avg_stagnation_length << ","
        << s.steps_since_last_best << ","
        << s.distinct_vars_flipped << ","
        << s.immediate_backflips << ","
        << s.flip_entropy << ","
        << s.normalized_flip_entropy << ","
        << s.most_flipped_var_ratio << ","
        << s.avg_revisit_distance << ","
        << s.distinct_unsat_clauses_seen << ","
        << s.unsat_clause_entropy << ","
        << s.normalized_unsat_clause_entropy << ","
        << s.most_frequent_unsat_clause_ratio << ","
        << s.avg_unsat_clause_lifetime << ","
        << s.max_unsat_clause_lifetime << ","
        << s.avg_hamming_from_initial << ","
        << s.max_hamming_from_initial << ","
        << s.area_under_unsat_curve << ","
        << s.normalized_auc << ","
        << s.early_progress_rate << ","
        << s.late_progress_rate << ","
        << s.time_to_50pct_improvement << ","
        << s.time_to_90pct_improvement << ","
        << s.restarts << ","
        << s.successful_try << ","
        << s.avg_best_unsat_per_try << ","
        << s.std_best_unsat_per_try << ","
        << s.paws_penalty_events << ","
        << s.paws_weight_increments << ","
        << s.paws_smoothing_events << ","
        << s.paws_weight_decrements << ","
        << s.paws_final_weighted_clauses << ","
        << s.paws_max_weight_seen << ","
        << s.paws_final_max_weight << ","
        << s.paws_final_mean_weight << ","
        << s.paws_avg_weighted_clauses << ","
        << s.paws_avg_max_weight
        << "\n";

    instance_stream.flush();
}

void CSVLogger::logSummaryResult(
    const SummaryResult& result)
{
    summary_stream
        << result.solver << ","
        << result.instance_type << ","
        << result.q << ","
        << result.n << ","
        << result.ratio << ","
		<< result.seed << ","
		<< std::fixed << std::setprecision(6)
		<< result.p << ","
		<< result.maxinc << ","
		<< result.alpha << ","
		<< result.lambda << ","
        << result.bridge_clause_probability << ","
        << result.total_instances << ","
        << result.solved_instances << ","
        << result.reported_solved_instances << ","
        << result.invalid_solution_instances << ","
        << result.success_rate << ","
        << result.reported_success_rate << ","
        << result.invalid_solution_rate << ","
        << result.mean_runtime_ms << ","
        << result.mean_flips_used << ","
        << result.mean_best_unsat << ","
        << result.mean_final_unsat << ","
        << result.mean_normalized_auc << ","
        << result.mean_longest_stagnation << ","
        << result.mean_flip_entropy << ","
        << result.mean_random_steps << ","
        << result.mean_random_improving_steps << ","
        << result.mean_random_neutral_steps << ","
        << result.mean_random_worsening_steps << ","
        << result.mean_heuristic_steps << ","
        << result.mean_heuristic_improving_steps << ","
        << result.mean_heuristic_neutral_steps << ","
        << result.mean_heuristic_worsening_steps << ","
        << result.mean_unknown_steps << ","
        << result.mean_unknown_improving_steps << ","
        << result.mean_unknown_neutral_steps << ","
        << result.mean_unknown_worsening_steps << ","
        << result.mean_paws_penalty_events << ","
        << result.mean_paws_weight_increments << ","
        << result.mean_paws_smoothing_events << ","
        << result.mean_paws_weight_decrements << ","
        << result.mean_paws_final_weighted_clauses << ","
        << result.mean_paws_max_weight_seen << ","
        << result.mean_paws_final_max_weight << ","
        << result.mean_paws_final_mean_weight << ","
        << result.mean_paws_avg_weighted_clauses << ","
        << result.mean_paws_avg_max_weight
        << "\n";

    summary_stream.flush();
}

void CSVLogger::logAutoStoppingEvent(
    const AutoStoppingEvent& event)
{
    auto_stopping_stream
        << event.event << ","
        << event.instance_type << ","
        << event.q << ","
        << event.n << ","
        << event.ratio << ","
        << event.solver << ","
        << event.solved_count << ","
        << event.fail_streak << ","
        << event.patience_ratios
        << "\n";

    auto_stopping_stream.flush();
}
