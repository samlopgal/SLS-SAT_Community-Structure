// ============================================================
// WalkSAT_Bridge_Break_PAWS_DET.h
// ============================================================

#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "ISolver.h"
#include "PrecomputedCommunityData.h"

class WalkSAT_Bridge_Break_PAWS_DET : public ISolver {
public:

    WalkSAT_Bridge_Break_PAWS_DET(
        const std::vector<std::vector<int>>& formula,
        int num_variables,
        int seed = 1,
        const PrecomputedCommunityData* community_data = nullptr,
        int maxinc = 20
    );

    bool solve(
        int max_flips,
        int max_tries,
        double p,
        SolverObserver* observer = nullptr
    ) override;

    const std::vector<uint8_t>& getAssignment() const override;

private:

    // ========================================================
    // Formula
    // ========================================================

    const std::vector<std::vector<int>>& formula;

    int num_variables;

    const PrecomputedCommunityData* community_data_;

    // ========================================================
    // RNG
    // ========================================================

    std::mt19937 rng_;

    // PAWS deterministic smoothing: after maxinc penalty events,
    // all structure-relevant clause weights greater than 1 are decremented by 1.
    // If maxinc <= 0, smoothing is disabled.
    int maxinc_;

    int randInt(int low, int high);

    double randDouble();

    // ========================================================
    // Assignment
    // ========================================================

    std::vector<uint8_t> assignment;

    // ========================================================
    // Clause state
    // ========================================================

    std::vector<int> true_literal_count;

    // Variable that uniquely satisfies a clause.
    // -1 means no unique true variable.
    std::vector<int> critical_variable;

    // ========================================================
    // Occurrences
    // ========================================================

    struct Occurrence {
        int clause;
        bool positive;
    };

    std::vector<std::vector<Occurrence>> variable_occurrences;

    // ========================================================
    // Unsatisfied clauses
    // ========================================================

    std::vector<int> unsatisfied_clauses;

    std::vector<int> clause_position;

    // ========================================================
    // Bridge clauses
    // ========================================================

    std::vector<uint8_t> is_bridge_clause_;

    // ========================================================
    // PAWS weights
    // ========================================================

    // Only bridge clauses are weighted by this solver.
    // Non-bridge clauses remain irrelevant for the structure-aware score.
    std::vector<int> clause_weight;

    // Incremental PAWS weight statistics.
    // These make maxClauseWeight() and meanClauseWeight() O(1)
    // instead of scanning all clauses on every flip.
    long long total_clause_weight = 0;
    int tracked_clause_count = 0;
    int current_max_clause_weight = 1;
    std::vector<int> weight_frequency;

    std::vector<int> weighted_clauses;

    std::vector<int> clause_position_in_weighted;

    // ========================================================
    // Incremental weighted heuristic
    // ========================================================

    // bridge_breakcount[v] =
    // weighted number of bridge clauses with true_literal_count == 1
    // and critical_variable == v.
    std::vector<int> bridge_breakcount;

    // ========================================================
    // Initialization
    // ========================================================

    void randomAssignment();

    void initializeVariableOccurrences();

    void initializeDataStructures();

    // ========================================================
    // Unsat tracking
    // ========================================================

    void updateUnsatisfiedClauses(
        int clause,
        bool unsatisfied
    );

    // ========================================================
    // PAWS weight handling
    // ========================================================

    inline int weight(int clause) const noexcept;

    void addWeightedClause(int clause);

    void removeWeightedClause(int clause);

    void resetWeightStatistics(int tracked_count);
    void noteWeightIncrease(int old_weight);
    void noteWeightDecrease(int old_weight);

    void increaseClauseWeight(int clause);

    void decreaseClauseWeight(int clause);

    int normalizeWeights();

    int countWeightedClauses() const;
    int maxClauseWeight() const;
    double meanClauseWeight() const;

    // ========================================================
    // Incremental updates
    // ========================================================

    void updateAfterFlip(int variable);

    int findUniqueTrueVariable(int clause) const noexcept;

    // ========================================================
    // Heuristic
    // ========================================================

    // Selects a variable from the chosen UNSAT clause using:
    // bridge_breakcount[v]
    int selectVariableToFlip(int clause);

    // ========================================================
    // Utils
    // ========================================================

    static inline int lit_to_var(int lit)
    {
        return (lit > 0) ? lit : -lit;
    }

    static inline bool lit_is_true(
        int lit,
        const std::vector<uint8_t>& assignment)
    {
        int var = lit_to_var(lit);

        return (lit > 0)
            ? assignment[var]
            : !assignment[var];
    }
};
