#include "Statistics.h"

void Statistics::add(const InstanceResult& result)
{
    GroupKey key;
    key.solver = result.solver;
    key.instance_type = result.instance_type;
    key.q = result.q;
    key.n = result.n;
    key.ratio = result.ratio;
    key.seed = result.seed;
    key.p = result.p;
    key.maxinc = result.maxinc;
    key.alpha = result.alpha;
    key.lambda = result.lambda;
    key.bridge_clause_probability = result.bridge_clause_probability;

    StatsAccumulator& acc = data_[key];

    ++acc.total_instances;

    if (result.verified_solved) {
        ++acc.solved_instances;
    }
    if (result.reported_solved) {
        ++acc.reported_solved_instances;
    }
    if (result.invalid_solution) {
        ++acc.invalid_solution_instances;
    }

    acc.total_runtime_ms += result.runtime_ms;
    acc.total_flips_used += static_cast<double>(result.stats.flips_used);
    acc.total_best_unsat += static_cast<double>(result.stats.best_unsat);
    acc.total_final_unsat += static_cast<double>(result.stats.final_unsat);
    acc.total_normalized_auc += result.stats.normalized_auc;
    acc.total_longest_stagnation += static_cast<double>(result.stats.longest_stagnation);
    acc.total_flip_entropy += result.stats.normalized_flip_entropy;

    acc.total_random_steps += static_cast<double>(result.stats.random_steps);
    acc.total_random_improving_steps += static_cast<double>(result.stats.random_improving_steps);
    acc.total_random_neutral_steps += static_cast<double>(result.stats.random_neutral_steps);
    acc.total_random_worsening_steps += static_cast<double>(result.stats.random_worsening_steps);

    acc.total_heuristic_steps += static_cast<double>(result.stats.heuristic_steps);
    acc.total_heuristic_improving_steps += static_cast<double>(result.stats.heuristic_improving_steps);
    acc.total_heuristic_neutral_steps += static_cast<double>(result.stats.heuristic_neutral_steps);
    acc.total_heuristic_worsening_steps += static_cast<double>(result.stats.heuristic_worsening_steps);

    acc.total_unknown_steps += static_cast<double>(result.stats.unknown_steps);
    acc.total_unknown_improving_steps += static_cast<double>(result.stats.unknown_improving_steps);
    acc.total_unknown_neutral_steps += static_cast<double>(result.stats.unknown_neutral_steps);
    acc.total_unknown_worsening_steps += static_cast<double>(result.stats.unknown_worsening_steps);

    acc.total_paws_penalty_events +=
        static_cast<double>(result.stats.paws_penalty_events);
    acc.total_paws_weight_increments +=
        static_cast<double>(result.stats.paws_weight_increments);
    acc.total_paws_smoothing_events +=
        static_cast<double>(result.stats.paws_smoothing_events);
    acc.total_paws_weight_decrements +=
        static_cast<double>(result.stats.paws_weight_decrements);
    acc.total_paws_final_weighted_clauses +=
        static_cast<double>(result.stats.paws_final_weighted_clauses);
    acc.total_paws_max_weight_seen +=
        static_cast<double>(result.stats.paws_max_weight_seen);
    acc.total_paws_final_max_weight +=
        static_cast<double>(result.stats.paws_final_max_weight);
    acc.total_paws_final_mean_weight +=
        result.stats.paws_final_mean_weight;
    acc.total_paws_avg_weighted_clauses +=
        result.stats.paws_avg_weighted_clauses;
    acc.total_paws_avg_max_weight +=
        result.stats.paws_avg_max_weight;
}

void Statistics::merge(const Statistics& other)
{
    for (const auto& [key, other_acc] : other.data_) {
        StatsAccumulator& acc = data_[key];

        acc.total_instances += other_acc.total_instances;
        acc.solved_instances += other_acc.solved_instances;
        acc.reported_solved_instances += other_acc.reported_solved_instances;
        acc.invalid_solution_instances += other_acc.invalid_solution_instances;

        acc.total_runtime_ms += other_acc.total_runtime_ms;
        acc.total_flips_used += other_acc.total_flips_used;
        acc.total_best_unsat += other_acc.total_best_unsat;
        acc.total_final_unsat += other_acc.total_final_unsat;
        acc.total_normalized_auc += other_acc.total_normalized_auc;
        acc.total_longest_stagnation += other_acc.total_longest_stagnation;
        acc.total_flip_entropy += other_acc.total_flip_entropy;

        acc.total_random_steps += other_acc.total_random_steps;
        acc.total_random_improving_steps += other_acc.total_random_improving_steps;
        acc.total_random_neutral_steps += other_acc.total_random_neutral_steps;
        acc.total_random_worsening_steps += other_acc.total_random_worsening_steps;

        acc.total_heuristic_steps += other_acc.total_heuristic_steps;
        acc.total_heuristic_improving_steps += other_acc.total_heuristic_improving_steps;
        acc.total_heuristic_neutral_steps += other_acc.total_heuristic_neutral_steps;
        acc.total_heuristic_worsening_steps += other_acc.total_heuristic_worsening_steps;

        acc.total_unknown_steps += other_acc.total_unknown_steps;
        acc.total_unknown_improving_steps += other_acc.total_unknown_improving_steps;
        acc.total_unknown_neutral_steps += other_acc.total_unknown_neutral_steps;
        acc.total_unknown_worsening_steps += other_acc.total_unknown_worsening_steps;

        acc.total_paws_penalty_events += other_acc.total_paws_penalty_events;
        acc.total_paws_weight_increments += other_acc.total_paws_weight_increments;
        acc.total_paws_smoothing_events += other_acc.total_paws_smoothing_events;
        acc.total_paws_weight_decrements += other_acc.total_paws_weight_decrements;
        acc.total_paws_final_weighted_clauses += other_acc.total_paws_final_weighted_clauses;
        acc.total_paws_max_weight_seen += other_acc.total_paws_max_weight_seen;
        acc.total_paws_final_max_weight += other_acc.total_paws_final_max_weight;
        acc.total_paws_final_mean_weight += other_acc.total_paws_final_mean_weight;
        acc.total_paws_avg_weighted_clauses += other_acc.total_paws_avg_weighted_clauses;
        acc.total_paws_avg_max_weight += other_acc.total_paws_avg_max_weight;
    }
}

std::vector<SummaryResult> Statistics::computeSummary() const
{
    std::vector<SummaryResult> results;
    results.reserve(data_.size());

    for (const auto& [key, acc] : data_) {
        SummaryResult summary;

        summary.solver = key.solver;
        summary.instance_type = key.instance_type;
        summary.q = key.q;
        summary.n = key.n;
        summary.ratio = key.ratio;
        summary.seed = key.seed;
        summary.p = key.p;
        summary.maxinc = key.maxinc;
        summary.alpha = key.alpha;
        summary.lambda = key.lambda;
        summary.bridge_clause_probability = key.bridge_clause_probability;

        summary.total_instances = acc.total_instances;
        summary.solved_instances = acc.solved_instances;
        summary.reported_solved_instances = acc.reported_solved_instances;
        summary.invalid_solution_instances = acc.invalid_solution_instances;

        if (acc.total_instances > 0) {
            const double total = static_cast<double>(acc.total_instances);

            summary.success_rate = static_cast<double>(acc.solved_instances) / total;
            summary.reported_success_rate = static_cast<double>(acc.reported_solved_instances) / total;
            summary.invalid_solution_rate = static_cast<double>(acc.invalid_solution_instances) / total;

            summary.mean_runtime_ms = acc.total_runtime_ms / total;
            summary.mean_flips_used = acc.total_flips_used / total;
            summary.mean_best_unsat = acc.total_best_unsat / total;
            summary.mean_final_unsat = acc.total_final_unsat / total;
            summary.mean_normalized_auc = acc.total_normalized_auc / total;
            summary.mean_longest_stagnation = acc.total_longest_stagnation / total;
            summary.mean_flip_entropy = acc.total_flip_entropy / total;

            summary.mean_random_steps = acc.total_random_steps / total;
            summary.mean_random_improving_steps = acc.total_random_improving_steps / total;
            summary.mean_random_neutral_steps = acc.total_random_neutral_steps / total;
            summary.mean_random_worsening_steps = acc.total_random_worsening_steps / total;

            summary.mean_heuristic_steps = acc.total_heuristic_steps / total;
            summary.mean_heuristic_improving_steps = acc.total_heuristic_improving_steps / total;
            summary.mean_heuristic_neutral_steps = acc.total_heuristic_neutral_steps / total;
            summary.mean_heuristic_worsening_steps = acc.total_heuristic_worsening_steps / total;

            summary.mean_unknown_steps = acc.total_unknown_steps / total;
            summary.mean_unknown_improving_steps = acc.total_unknown_improving_steps / total;
            summary.mean_unknown_neutral_steps = acc.total_unknown_neutral_steps / total;
            summary.mean_unknown_worsening_steps = acc.total_unknown_worsening_steps / total;

            summary.mean_paws_penalty_events =
                acc.total_paws_penalty_events / total;
            summary.mean_paws_weight_increments =
                acc.total_paws_weight_increments / total;
            summary.mean_paws_smoothing_events =
                acc.total_paws_smoothing_events / total;
            summary.mean_paws_weight_decrements =
                acc.total_paws_weight_decrements / total;
            summary.mean_paws_final_weighted_clauses =
                acc.total_paws_final_weighted_clauses / total;
            summary.mean_paws_max_weight_seen =
                acc.total_paws_max_weight_seen / total;
            summary.mean_paws_final_max_weight =
                acc.total_paws_final_max_weight / total;
            summary.mean_paws_final_mean_weight =
                acc.total_paws_final_mean_weight / total;
            summary.mean_paws_avg_weighted_clauses =
                acc.total_paws_avg_weighted_clauses / total;
            summary.mean_paws_avg_max_weight =
                acc.total_paws_avg_max_weight / total;
        }

        results.push_back(summary);
    }

    return results;
}
