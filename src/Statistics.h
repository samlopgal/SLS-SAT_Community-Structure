#pragma once

#include <map>
#include <string>
#include <vector>

#include "CSVLogger.h"

struct GroupKey {
    std::string solver;
    std::string instance_type;
    int q = 0;
    int n = 0;
    int ratio = 0;
    int seed = 0;
    double p = 0.0;
    int maxinc = 0;
    double alpha = 1.0;
    double lambda = 0.0;
    double bridge_clause_probability = 0.0;

    bool operator<(const GroupKey& other) const {
        if (solver != other.solver) return solver < other.solver;
        if (instance_type != other.instance_type) return instance_type < other.instance_type;
        if (q != other.q) return q < other.q;
        if (n != other.n) return n < other.n;
        if (ratio != other.ratio) return ratio < other.ratio;
        if (seed != other.seed) return seed < other.seed;
        if (p != other.p) return p < other.p;
        if (maxinc != other.maxinc) return maxinc < other.maxinc;
        if (alpha != other.alpha) return alpha < other.alpha;
        if (lambda != other.lambda) return lambda < other.lambda;
        return bridge_clause_probability < other.bridge_clause_probability;
    }
};

struct StatsAccumulator {
    int total_instances = 0;
    int solved_instances = 0;
    int reported_solved_instances = 0;
    int invalid_solution_instances = 0;

    double total_runtime_ms = 0.0;
    double total_flips_used = 0.0;
    double total_best_unsat = 0.0;
    double total_final_unsat = 0.0;
    double total_normalized_auc = 0.0;
    double total_longest_stagnation = 0.0;
    double total_flip_entropy = 0.0;

    double total_random_steps = 0.0;
    double total_random_improving_steps = 0.0;
    double total_random_neutral_steps = 0.0;
    double total_random_worsening_steps = 0.0;

    double total_heuristic_steps = 0.0;
    double total_heuristic_improving_steps = 0.0;
    double total_heuristic_neutral_steps = 0.0;
    double total_heuristic_worsening_steps = 0.0;

    double total_unknown_steps = 0.0;
    double total_unknown_improving_steps = 0.0;
    double total_unknown_neutral_steps = 0.0;
    double total_unknown_worsening_steps = 0.0;

    double total_paws_penalty_events = 0.0;
    double total_paws_weight_increments = 0.0;
    double total_paws_smoothing_events = 0.0;
    double total_paws_weight_decrements = 0.0;
    double total_paws_final_weighted_clauses = 0.0;
    double total_paws_max_weight_seen = 0.0;
    double total_paws_final_max_weight = 0.0;
    double total_paws_final_mean_weight = 0.0;
    double total_paws_avg_weighted_clauses = 0.0;
    double total_paws_avg_max_weight = 0.0;
};

class Statistics {
public:
    void add(const InstanceResult& result);
    void merge(const Statistics& other);
    std::vector<SummaryResult> computeSummary() const;

private:
    std::map<GroupKey, StatsAccumulator> data_;
};
