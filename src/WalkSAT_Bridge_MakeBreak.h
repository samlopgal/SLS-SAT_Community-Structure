// ============================================================
// WalkSAT_Bridge_MakeBreak.h
// ============================================================

#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "ISolver.h"
#include "PrecomputedCommunityData.h"

class WalkSAT_Bridge_MakeBreak : public ISolver {
public:

    WalkSAT_Bridge_MakeBreak(
        const std::vector<std::vector<int>>& formula,
        int num_variables,
        int seed = 1,
        const PrecomputedCommunityData* community_data = nullptr
    );

    bool solve(
        int max_flips,
        int max_tries,
        double p,
        SolverObserver* observer = nullptr
    ) override;

    const std::vector<uint8_t>&
    getAssignment() const override;

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

    // variable that uniquely satisfies clause
    std::vector<int> critical_variable;

    // ========================================================
    // Occurrences
    // ========================================================

    struct Occurrence {
        int clause;
        bool positive;
    };

    std::vector<std::vector<Occurrence>>
        variable_occurrences;

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
    // Incremental heuristic
    // ========================================================

    // bridge_makecount[v] =
    // nº de cláusulas puente UNSAT que v puede satisfacer
    std::vector<int> bridge_makecount;

    // bridge_breakcount[v] =
    // nº de cláusulas puente con true_literal_count == 1
    // y critical_variable == v
    std::vector<int> bridge_breakcount;

    // ========================================================
    // Initialization
    // ========================================================

    void randomAssignment();

    void initializeVariableOccurrences();

    void initializeDataStructures();

    // ========================================================
    // Incremental updates
    // ========================================================

    void updateUnsatisfiedClauses(
        int clause,
        bool unsatisfied
    );

    void updateAfterFlip(
        int variable
    );

    int findUniqueTrueVariable(
        int clause
    ) const noexcept;

    void addBridgeMakeContribution(
        int clause
    );

    void removeBridgeMakeContribution(
        int clause
    );

    // ========================================================
    // Heuristic
    // ========================================================

    int selectVariableToFlip(
        int clause
    );

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
