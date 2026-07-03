#pragma once

#include "SolverObserver.h"

#include <cstdint>
#include <vector>

class GenericStatsCollector : public SolverObserver {
public:
    GenericStatsCollector(
        int num_variables,
        int num_clauses
    );

    void onTryStart(
        int try_id,
        int initial_unsat,
        const std::vector<int>& unsat_clauses,
        const std::vector<uint8_t>& assignment
    ) override;

    void onFlip(
        int try_id,
        int selected_var_dimacs,
        int old_unsat,
        int new_unsat,
        const std::vector<int>& unsat_clauses,
        const std::vector<uint8_t>& assignment,
        MoveOrigin origin = MoveOrigin::Unknown
    ) override;

    void onTryEnd(
        int try_id,
        bool solved,
        int final_unsat
    ) override;

    void onSolveEnd(
        bool reported_solved,
        int final_unsat
    ) override;

    void onPawsStep(
        int try_id,
        int flip,
        bool penalty_applied,
        int weight_increments,
        bool smoothing_applied,
        int weight_decrements,
        int weighted_clauses,
        int max_clause_weight,
        double mean_clause_weight
    ) override;

    const GenericSolverStats& stats() const;
    GenericSolverStats& mutableStats();

private:
    int num_variables_ = 0;
    int num_clauses_ = 0;

    GenericSolverStats stats_;

    bool has_any_try_ = false;
    int current_try_best_unsat_ = 0;
    long long current_try_flips_ = 0;

    std::vector<int> best_unsat_by_try_;

    std::vector<long long> var_flip_count_;       // índice 1..num_variables
    std::vector<long long> last_flip_time_;       // índice 1..num_variables
    long long revisit_distance_sum_ = 0;
    long long revisit_events_ = 0;
    int previous_var_ = -1;

    std::vector<long long> unsat_seen_count_;     // índice cláusula 0..m-1
    std::vector<long long> unsat_lifetime_;       // tiempo acumulado como insat
    std::vector<long long> unsat_start_flip_;     // -1 si no está insat ahora

    std::vector<uint8_t> initial_assignment_;
    int current_hamming_from_initial_ = 0;
    double hamming_sum_ = 0.0;
    long long hamming_samples_ = 0;

    long long delta_sum_ = 0;
    long long abs_delta_sum_ = 0;

    long long current_stagnation_ = 0;
    long long stagnation_length_sum_ = 0;

    bool reached_50pct_ = false;
    bool reached_90pct_ = false;

    long long paws_weighted_clause_sum_ = 0;
    long long paws_max_weight_sum_ = 0;
    long long paws_observed_steps_ = 0;

    int assignmentIndexForVar(
        int var_dimacs,
        const std::vector<uint8_t>& assignment
    ) const;

    void accountUnsatClauses(
        const std::vector<int>& unsat_clauses
    );

    void closeUnsatLifetimes();
    void finalizeDerivedStats();
};
