// ============================================================
// WalkSAT_Community_Break_PAWS_DET.cpp
// ============================================================

#include "WalkSAT_Community_Break_PAWS_DET.h"

#include <algorithm>
#include <climits>

// ============================================================
// Constructor
// ============================================================

WalkSAT_Community_Break_PAWS_DET::
WalkSAT_Community_Break_PAWS_DET(
    const std::vector<std::vector<int>>& formula,
    int num_variables,
    int seed,
    const PrecomputedCommunityData* community_data,
    int maxinc)
    :
    formula(formula),
    num_variables(num_variables),
    community_data_(community_data),
    rng_(seed),
    maxinc_(maxinc)
{
    assignment.resize(num_variables + 1, 0);

    initializeVariableOccurrences();

    is_community_clause_.assign(
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
                    .size() == 1)
            {
                is_community_clause_[c] = 1;
            }
        }
    }

    clause_weight.assign(
        formula.size(),
        1
    );

    clause_position_in_weighted.assign(
        formula.size(),
        -1
    );

    weighted_clauses.reserve(
        formula.size()
    );

    initializeDataStructures();
}

// ============================================================
// RNG
// ============================================================

int WalkSAT_Community_Break_PAWS_DET::
randInt(int low, int high)
{
    std::uniform_int_distribution<int>
        dist(low, high);

    return dist(rng_);
}

double WalkSAT_Community_Break_PAWS_DET::
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
WalkSAT_Community_Break_PAWS_DET::
getAssignment() const
{
    return assignment;
}

void WalkSAT_Community_Break_PAWS_DET::
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

void WalkSAT_Community_Break_PAWS_DET::
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

void WalkSAT_Community_Break_PAWS_DET::
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

    community_breakcount.assign(
        num_variables + 1,
        0
    );

    std::fill(
        clause_weight.begin(),
        clause_weight.end(),
        1
    );


    int relevant_clause_count = 0;
    for (uint8_t is_relevant : is_community_clause_) {
        if (is_relevant) {
            ++relevant_clause_count;
        }
    }
    resetWeightStatistics(relevant_clause_count);

    std::fill(
        clause_position_in_weighted.begin(),
        clause_position_in_weighted.end(),
        -1
    );

    weighted_clauses.clear();

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

            if (is_community_clause_[c] &&
                critical != -1)
            {
                community_breakcount[critical] +=
                    weight(
                        static_cast<int>(c)
                    );
            }
        }

        if (count == 0) {

            clause_position[c] =
                static_cast<int>(
                    unsatisfied_clauses.size()
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

void WalkSAT_Community_Break_PAWS_DET::
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
// PAWS weights
// ============================================================

inline int WalkSAT_Community_Break_PAWS_DET::
weight(int clause) const noexcept
{
    return clause_weight[clause];
}



void WalkSAT_Community_Break_PAWS_DET::resetWeightStatistics(int tracked_count)
{
    tracked_clause_count = tracked_count;
    total_clause_weight = static_cast<long long>(tracked_count);
    current_max_clause_weight = 1;

    weight_frequency.assign(2, 0);
    if (tracked_count > 0) {
        weight_frequency[1] = tracked_count;
    }
}

void WalkSAT_Community_Break_PAWS_DET::noteWeightIncrease(int old_weight)
{
    const int new_weight = old_weight + 1;

    if (old_weight >= static_cast<int>(weight_frequency.size())) {
        weight_frequency.resize(old_weight + 1, 0);
    }
    --weight_frequency[old_weight];

    if (new_weight >= static_cast<int>(weight_frequency.size())) {
        weight_frequency.resize(new_weight + 1, 0);
    }
    ++weight_frequency[new_weight];

    ++total_clause_weight;

    if (new_weight > current_max_clause_weight) {
        current_max_clause_weight = new_weight;
    }
}

void WalkSAT_Community_Break_PAWS_DET::noteWeightDecrease(int old_weight)
{
    if (old_weight <= 1) {
        return;
    }

    const int new_weight = old_weight - 1;

    if (old_weight >= static_cast<int>(weight_frequency.size())) {
        weight_frequency.resize(old_weight + 1, 0);
    }
    --weight_frequency[old_weight];
    ++weight_frequency[new_weight];

    --total_clause_weight;

    while (current_max_clause_weight > 1 &&
           current_max_clause_weight < static_cast<int>(weight_frequency.size()) &&
           weight_frequency[current_max_clause_weight] == 0)
    {
        --current_max_clause_weight;
    }
}

void WalkSAT_Community_Break_PAWS_DET::
addWeightedClause(int clause)
{
    if (clause_position_in_weighted[clause] != -1) {
        return;
    }

    clause_position_in_weighted[clause] =
        static_cast<int>(
            weighted_clauses.size()
        );

    weighted_clauses.push_back(clause);
}

void WalkSAT_Community_Break_PAWS_DET::
removeWeightedClause(int clause)
{
    int pos =
        clause_position_in_weighted[clause];

    if (pos == -1) {
        return;
    }

    int last =
        weighted_clauses.back();

    weighted_clauses[pos] =
        last;

    clause_position_in_weighted[last] =
        pos;

    weighted_clauses.pop_back();

    clause_position_in_weighted[clause] =
        -1;
}

void WalkSAT_Community_Break_PAWS_DET::
increaseClauseWeight(int clause)
{
    if (!is_community_clause_[clause]) {
        return;
    }

    int old_weight =
        clause_weight[clause];

    if (old_weight == 1) {
        addWeightedClause(clause);
    }

    if (true_literal_count[clause] == 1 &&
        critical_variable[clause] != -1)
    {
        ++community_breakcount[
            critical_variable[clause]
        ];
    }

    noteWeightIncrease(old_weight);

    ++clause_weight[clause];
}

void WalkSAT_Community_Break_PAWS_DET::
decreaseClauseWeight(int clause)
{
    if (!is_community_clause_[clause]) {
        return;
    }

    int old_weight =
        clause_weight[clause];

    if (old_weight <= 1) {
        return;
    }

    if (true_literal_count[clause] == 1 &&
        critical_variable[clause] != -1)
    {
        --community_breakcount[
            critical_variable[clause]
        ];
    }

    noteWeightDecrease(old_weight);

    --clause_weight[clause];

    if (clause_weight[clause] == 1) {
        removeWeightedClause(clause);
    }
}

int WalkSAT_Community_Break_PAWS_DET::normalizeWeights()
{
    int decrements = 0;
    int i = 0;

    while (i < static_cast<int>(weighted_clauses.size())) {
        int clause = weighted_clauses[i];
        int before = clause_weight[clause];

        decreaseClauseWeight(clause);

        if (clause_weight[clause] < before) {
            ++decrements;
        }

        if (i < static_cast<int>(weighted_clauses.size()) &&
            weighted_clauses[i] == clause) {
            ++i;
        }
    }

    return decrements;
}


int WalkSAT_Community_Break_PAWS_DET::countWeightedClauses() const
{
    return static_cast<int>(weighted_clauses.size());
}

int WalkSAT_Community_Break_PAWS_DET::maxClauseWeight() const
{
    return current_max_clause_weight;
}

double WalkSAT_Community_Break_PAWS_DET::meanClauseWeight() const
{
    if (tracked_clause_count == 0) {
        return 1.0;
    }

    return static_cast<double>(total_clause_weight) /
           static_cast<double>(tracked_clause_count);
}


// ============================================================
// Unique true variable
// ============================================================

int WalkSAT_Community_Break_PAWS_DET::
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

void WalkSAT_Community_Break_PAWS_DET::
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
        // Remove old weighted contributions
        // ----------------------------------------------------

        if (is_community_clause_[clause]) {

            if (before == 1 &&
                old_critical != -1)
            {
                community_breakcount[old_critical] -=
                    weight(clause);
            }
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
        // Update critical variable
        // ----------------------------------------------------

        int new_critical = -1;

        if (after == 1) {

            if (!literal_before &&
                 literal_after)
            {
                new_critical =
                    variable;
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
        // Add new weighted contributions
        // ----------------------------------------------------

        if (is_community_clause_[clause]) {

            if (after == 1 &&
                new_critical != -1)
            {
                community_breakcount[new_critical] +=
                    weight(clause);
            }
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

int WalkSAT_Community_Break_PAWS_DET::
selectVariableToFlip(
    int clause)
{
    const auto& c = formula[clause];

    std::vector<int> candidates;

    int first_var =
        lit_to_var(c[0]);

    int best_community_break =
        community_breakcount[first_var];

    candidates.push_back(
        first_var
    );

    for (std::size_t i = 1;
         i < c.size();
         ++i)
    {
        int var =
            lit_to_var(c[i]);

        int score =
            community_breakcount[var];

        if (score < best_community_break) {

            best_community_break =
                score;

            candidates.clear();

            candidates.push_back(
                var
            );
        }
        else if (
            score == best_community_break)
        {
            candidates.push_back(
                var
            );
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

bool WalkSAT_Community_Break_PAWS_DET::
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
                static_cast<int>(
                    unsatisfied_clauses.size()
                ),
                unsatisfied_clauses,
                assignment
            );
        }

        int previous_unsat =
            static_cast<int>(
                unsatisfied_clauses.size()
            );

        int weight_update_events = 0;

        for (int flip = 0;
             flip < max_flips;
             ++flip)
        {
            if (unsatisfied_clauses.empty()) {

                if (observer) {
                    observer->onTryEnd(
                        t + 1,
                        true,
                        0
                    );

                    observer->onSolveEnd(
                        true,
                        0
                    );
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

            const int current_unsat =
                static_cast<int>(
                    unsatisfied_clauses.size()
                );

            // ------------------------------------------------
            // PAWS update:
            // penalize only currently UNSAT structural clauses
            // when the move did not improve the number of UNSAT
            // clauses.
            // ------------------------------------------------

            bool paws_penalty_applied = false;
            int paws_weight_increments = 0;
            bool paws_smoothing_applied = false;
            int paws_weight_decrements = 0;

            if (current_unsat >= previous_unsat) {

                for (int ci : unsatisfied_clauses) {

                    if (is_community_clause_[ci]) {
                        increaseClauseWeight(ci);
                        ++paws_weight_increments;
                    }
                }

                paws_penalty_applied = (paws_weight_increments > 0);

                if (paws_penalty_applied) {
                    ++weight_update_events;

                    if (maxinc_ > 0 && weight_update_events % maxinc_ == 0) {
                        paws_weight_decrements = normalizeWeights();
                        paws_smoothing_applied = (paws_weight_decrements > 0);
                    }
                }
            }

            if (observer) {
                observer->onPawsStep(
                    t + 1,
                    flip + 1,
                    paws_penalty_applied,
                    paws_weight_increments,
                    paws_smoothing_applied,
                    paws_weight_decrements,
                    countWeightedClauses(),
                    maxClauseWeight(),
                    meanClauseWeight()
                );
            }

            previous_unsat =
                current_unsat;

            if (observer) {
                observer->onFlip(
                    t + 1,
                    variable,
                    old_unsat,
                    current_unsat,
                    unsatisfied_clauses,
                    assignment,
                    origin
                );
            }

            if (unsatisfied_clauses.empty()) {

                if (observer) {
                    observer->onTryEnd(
                        t + 1,
                        true,
                        0
                    );

                    observer->onSolveEnd(
                        true,
                        0
                    );
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
