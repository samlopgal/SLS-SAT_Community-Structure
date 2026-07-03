// ============================================================
// WalkSAT_Bridge_Break.cpp
// ============================================================

#include "WalkSAT_Bridge_Break.h"

#include <algorithm>
#include <climits>

// ============================================================
// Constructor
// ============================================================

WalkSAT_Bridge_Break::
WalkSAT_Bridge_Break(
    const std::vector<std::vector<int>>& formula,
    int num_variables,
    int seed,
    const PrecomputedCommunityData* community_data)
    :
    formula(formula),
    num_variables(num_variables),
    community_data_(community_data),
    rng_(seed)
{
    assignment.resize(num_variables + 1, 0);

    initializeVariableOccurrences();

    is_bridge_clause_.assign(
        formula.size(),
        0
    );

    if (community_data_) {

        for (std::size_t c = 0;
             c < formula.size();
             ++c)
        {
            if (community_data_
                    ->clause_communities[c]
                    .size() > 1)
            {
                is_bridge_clause_[c] = 1;
            }
        }
    }

    initializeDataStructures();
}

// ============================================================
// RNG
// ============================================================

int WalkSAT_Bridge_Break::
randInt(int low, int high)
{
    std::uniform_int_distribution<int>
        dist(low, high);

    return dist(rng_);
}

double WalkSAT_Bridge_Break::
randDouble()
{
    std::uniform_real_distribution<double>
        dist(0.0, 1.0);

    return dist(rng_);
}

// ============================================================
// Assignment
// ============================================================

const std::vector<uint8_t>&
WalkSAT_Bridge_Break::
getAssignment() const
{
    return assignment;
}

void WalkSAT_Bridge_Break::
randomAssignment()
{
    for (int v = 1;
         v <= num_variables;
         ++v)
    {
        assignment[v] =
            static_cast<uint8_t>(
                randInt(0, 1)
            );
    }
}

// ============================================================
// Occurrences
// ============================================================

void WalkSAT_Bridge_Break::
initializeVariableOccurrences()
{
    variable_occurrences.assign(
        num_variables + 1,
        {}
    );

    for (std::size_t c = 0;
         c < formula.size();
         ++c)
    {
        for (int lit : formula[c]) {

            variable_occurrences[
                lit_to_var(lit)
            ].push_back({
                static_cast<int>(c),
                lit > 0
            });
        }
    }
}

// ============================================================
// Initialization
// ============================================================

void WalkSAT_Bridge_Break::
initializeDataStructures()
{
    true_literal_count.assign(
        formula.size(),
        0
    );

    critical_variable.assign(
        formula.size(),
        -1
    );

    clause_position.assign(
        formula.size(),
        -1
    );

    bridge_breakcount.assign(
        num_variables + 1,
        0
    );

    unsatisfied_clauses.clear();

    for (std::size_t c = 0;
         c < formula.size();
         ++c)
    {
        int count = 0;
        int critical = -1;

        for (int lit : formula[c]) {

            if (lit_is_true(
                    lit,
                    assignment))
            {
                ++count;

                critical =
                    lit_to_var(lit);
            }
        }

        true_literal_count[c] = count;

        if (count == 1) {
            critical_variable[c] =
                critical;

            if (is_bridge_clause_[c] &&
                critical != -1)
            {
                ++bridge_breakcount[critical];
            }
        }

        if (count == 0) {

            clause_position[c] =
                static_cast<int>(
                    unsatisfied_clauses
                        .size()
                );

            unsatisfied_clauses.push_back(
                static_cast<int>(c)
            );
        }
    }
}

// ============================================================
// Unsat clauses O(1)
// ============================================================

void WalkSAT_Bridge_Break::
updateUnsatisfiedClauses(
    int clause,
    bool unsatisfied)
{
    int pos =
        clause_position[clause];

    if (unsatisfied) {

        if (pos == -1) {

            clause_position[clause] =
                static_cast<int>(
                    unsatisfied_clauses
                        .size()
                );

            unsatisfied_clauses
                .push_back(clause);
        }
    }
    else {

        if (pos != -1) {

            int last =
                unsatisfied_clauses.back();

            unsatisfied_clauses[pos] =
                last;

            clause_position[last] =
                pos;

            unsatisfied_clauses.pop_back();

            clause_position[clause] =
                -1;
        }
    }
}

// ============================================================
// Unique true variable
// ============================================================

int WalkSAT_Bridge_Break::
findUniqueTrueVariable(
    int clause) const noexcept
{
    int true_var = -1;
    int count = 0;

    for (int lit : formula[clause]) {

        if (lit_is_true(
                lit,
                assignment))
        {
            true_var =
                lit_to_var(lit);

            ++count;

            if (count > 1) {
                return -1;
            }
        }
    }

    return (count == 1)
        ? true_var
        : -1;
}

// ============================================================
// Flip update
// ============================================================

void WalkSAT_Bridge_Break::
updateAfterFlip(
    int variable)
{
    assignment[variable] =
        !assignment[variable];

    for (const auto& occ :
         variable_occurrences[variable])
    {
        int clause = occ.clause;

        int before =
            true_literal_count[clause];

        int old_critical =
            critical_variable[clause];

        // ----------------------------------------------------
        // Quitar contribución antigua a bridge_breakcount
        // ----------------------------------------------------

        if (is_bridge_clause_[clause] &&
            before == 1 &&
            old_critical != -1)
        {
            --bridge_breakcount[old_critical];
        }

        bool literal_before =
            occ.positive
                ? !assignment[variable]
                : assignment[variable];

        bool literal_after =
            !literal_before;

        int after = before;

        if (!literal_before &&
             literal_after)
        {
            ++after;
        }
        else if (
            literal_before &&
            !literal_after)
        {
            --after;
        }

        true_literal_count[clause] =
            after;

        // ----------------------------------------------------
        // Actualizar critical_variable
        // ----------------------------------------------------

        int new_critical = -1;

        if (after == 1) {

            if (!literal_before &&
                 literal_after)
            {
                new_critical = variable;
            }
            else {
                new_critical =
                    findUniqueTrueVariable(
                        clause
                    );
            }
        }

        critical_variable[clause] =
            new_critical;

        // ----------------------------------------------------
        // Añadir nueva contribución a bridge_breakcount
        // ----------------------------------------------------

        if (is_bridge_clause_[clause] &&
            after == 1 &&
            new_critical != -1)
        {
            ++bridge_breakcount[new_critical];
        }

        // ----------------------------------------------------
        // Unsat tracking
        // ----------------------------------------------------

        if (before == 0 &&
            after > 0)
        {
            updateUnsatisfiedClauses(
                clause,
                false
            );
        }
        else if (
            before > 0 &&
            after == 0)
        {
            updateUnsatisfiedClauses(
                clause,
                true
            );
        }
    }
}

// ============================================================
// Variable selection
// ============================================================

int WalkSAT_Bridge_Break::
selectVariableToFlip(
    int clause)
{
    const auto& c = formula[clause];

    std::vector<int> candidates;

    int first_var =
        lit_to_var(c[0]);

    int best_bridge_break =
        bridge_breakcount[first_var];

    candidates.push_back(first_var);

    for (std::size_t i = 1;
         i < c.size();
         ++i)
    {
        int var =
            lit_to_var(c[i]);

        int bridge_break =
            bridge_breakcount[var];

        if (bridge_break < best_bridge_break) {

            best_bridge_break =
                bridge_break;

            candidates.clear();

            candidates.push_back(var);
        }
        else if (
            bridge_break ==
            best_bridge_break)
        {
            candidates.push_back(var);
        }
    }

    return candidates[
        randInt(
            0,
            static_cast<int>(
                candidates.size()
            ) - 1
        )
    ];
}

// ============================================================
// Solve
// ============================================================

bool WalkSAT_Bridge_Break::
solve(
    int max_flips,
    int max_tries,
    double p,
    SolverObserver* observer)
{
    for (int t = 0;
         t < max_tries;
         ++t)
    {
        randomAssignment();

        initializeDataStructures();

        if (observer) {
            observer->onTryStart(
                t + 1,
                static_cast<int>(unsatisfied_clauses.size()),
                unsatisfied_clauses,
                assignment
            );
        }

        for (int flip = 0;
             flip < max_flips;
             ++flip)
        {
            if (unsatisfied_clauses
                    .empty())
            {
                if (observer) {
                    observer->onTryEnd(t + 1, true, 0);
                    observer->onSolveEnd(true, 0);
                }
                return true;
            }

            int clause =
                unsatisfied_clauses[
                    randInt(
                        0,
                        static_cast<int>(
                            unsatisfied_clauses
                                .size()
                        ) - 1
                    )
                ];

            int variable;
            MoveOrigin origin = MoveOrigin::Unknown;

            if (randDouble() < p) {
                origin = MoveOrigin::Random;

                const auto& c =
                    formula[clause];

                variable =
                    lit_to_var(
                        c[randInt(
                            0,
                            static_cast<int>(
                                c.size()
                            ) - 1
                        )]
                    );
            }
            else {
                origin = MoveOrigin::Heuristic;
                variable =
                    selectVariableToFlip(
                        clause
                    );
            }

            const int old_unsat =
                static_cast<int>(
                    unsatisfied_clauses.size()
                );

            updateAfterFlip(variable);

            const int new_unsat =
                static_cast<int>(
                    unsatisfied_clauses.size()
                );

            if (observer) {
                observer->onFlip(
                    t + 1,
                    variable,
                    old_unsat,
                    new_unsat,
                    unsatisfied_clauses,
                    assignment,
                    origin
                );
            }

            if (unsatisfied_clauses
                    .empty())
            {
                if (observer) {
                    observer->onTryEnd(t + 1, true, 0);
                    observer->onSolveEnd(true, 0);
                }
                return true;
            }
        }

        if (observer) {
            observer->onTryEnd(
                t + 1,
                false,
                static_cast<int>(
                    unsatisfied_clauses.size()
                )
            );
        }
    }

    if (observer) {
        observer->onSolveEnd(
            false,
            static_cast<int>(
                unsatisfied_clauses.size()
            )
        );
    }

    return false;
}
