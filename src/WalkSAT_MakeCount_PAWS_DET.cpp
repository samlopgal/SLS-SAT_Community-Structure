#include "WalkSAT_MakeCount_PAWS_DET.h"
#include <algorithm>
#include <climits>
#include <random>

inline int WalkSAT_MakeCount_PAWS_DET::weight(int ci) const {
    return clause_weight[ci];
}



void WalkSAT_MakeCount_PAWS_DET::resetWeightStatistics(int tracked_count)
{
    tracked_clause_count = tracked_count;
    total_clause_weight = static_cast<long long>(tracked_count);
    current_max_clause_weight = 1;

    weight_frequency.assign(2, 0);
    if (tracked_count > 0) {
        weight_frequency[1] = tracked_count;
    }
}

void WalkSAT_MakeCount_PAWS_DET::noteWeightIncrease(int old_weight)
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

void WalkSAT_MakeCount_PAWS_DET::noteWeightDecrease(int old_weight)
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

WalkSAT_MakeCount_PAWS_DET::WalkSAT_MakeCount_PAWS_DET(
    const std::vector<std::vector<int>>& formula_,
    int num_variables_,
    int seed_,
    int maxinc_)
    : formula(formula_), num_variables(num_variables_), gen(seed_), maxinc(maxinc_)
{
    int m = formula.size();

    assignment.resize(num_variables);

    clause_sat_count.resize(m);
    clause_true_lit.resize(m);

    clause_weight.resize(m, 1);

    var_occ.resize(num_variables);

    clause_pos_in_unsat.resize(m);
    clause_pos_in_weighted.resize(m);

    breakcount.resize(num_variables);
    makecount.resize(num_variables);

    unsat_clauses.reserve(m);
    weighted_clauses.reserve(m);

    for (int c = 0; c < m; ++c) {
        for (int lit : formula[c]) {
            int v = (lit > 0 ? lit - 1 : -lit - 1);
            var_occ[v].push_back({c, lit > 0});
        }
    }
}

void WalkSAT_MakeCount_PAWS_DET::addUnsatClause(int ci) {
    clause_pos_in_unsat[ci] = unsat_clauses.size();
    unsat_clauses.push_back(ci);
}

void WalkSAT_MakeCount_PAWS_DET::removeUnsatClause(int ci) {
    int pos = clause_pos_in_unsat[ci];
    int last = unsat_clauses.back();

    unsat_clauses[pos] = last;
    clause_pos_in_unsat[last] = pos;

    unsat_clauses.pop_back();
    clause_pos_in_unsat[ci] = -1;
}

void WalkSAT_MakeCount_PAWS_DET::addWeightedClause(int ci) {
    clause_pos_in_weighted[ci] = weighted_clauses.size();
    weighted_clauses.push_back(ci);
}

void WalkSAT_MakeCount_PAWS_DET::removeWeightedClause(int ci) {
    int pos = clause_pos_in_weighted[ci];
    int last = weighted_clauses.back();

    weighted_clauses[pos] = last;
    clause_pos_in_weighted[last] = pos;

    weighted_clauses.pop_back();
    clause_pos_in_weighted[ci] = -1;
}

void WalkSAT_MakeCount_PAWS_DET::increaseClauseWeight(int ci) {
    int old_w = clause_weight[ci];

    if (old_w == 1)
        addWeightedClause(ci);

    noteWeightIncrease(old_w);

    clause_weight[ci]++;

    if (clause_sat_count[ci] == 0) {
        for (int lit : formula[ci]) {
            int v = (lit > 0 ? lit - 1 : -lit - 1);
            makecount[v]++;
        }
    }
    else if (clause_sat_count[ci] == 1 && clause_true_lit[ci] != -1) {
        breakcount[clause_true_lit[ci]]++;
    }
}

void WalkSAT_MakeCount_PAWS_DET::decreaseClauseWeight(int ci) {
    int old_w = clause_weight[ci];

    if (old_w <= 1)
        return;

    if (clause_sat_count[ci] == 0) {
        for (int lit : formula[ci]) {
            int v = (lit > 0 ? lit - 1 : -lit - 1);
            makecount[v]--;
        }
    }
    else if (clause_sat_count[ci] == 1 && clause_true_lit[ci] != -1) {
        breakcount[clause_true_lit[ci]]--;
    }

    noteWeightDecrease(old_w);

    clause_weight[ci]--;

    if (clause_weight[ci] == 1)
        removeWeightedClause(ci);
}

void WalkSAT_MakeCount_PAWS_DET::randomAssignment() {
    for (int i = 0; i < num_variables; ++i)
        assignment[i] = (gen() & 1);

    initializeClauseData();
}

void WalkSAT_MakeCount_PAWS_DET::initializeClauseData() {

    int m = formula.size();

    std::fill(clause_sat_count.begin(), clause_sat_count.end(), 0);
    std::fill(clause_true_lit.begin(), clause_true_lit.end(), -1);
    std::fill(clause_pos_in_unsat.begin(), clause_pos_in_unsat.end(), -1);
    std::fill(clause_pos_in_weighted.begin(), clause_pos_in_weighted.end(), -1);
    std::fill(breakcount.begin(), breakcount.end(), 0);
    std::fill(makecount.begin(), makecount.end(), 0);

    unsat_clauses.clear();
    weighted_clauses.clear();

    std::fill(clause_weight.begin(), clause_weight.end(), 1);
    resetWeightStatistics(static_cast<int>(formula.size()));

    for (int c = 0; c < m; ++c) {

        int true_var = -1;

        for (int lit : formula[c]) {
            int v = (lit > 0 ? lit - 1 : -lit - 1);
            bool val = assignment[v];

            if ((lit > 0 && val) || (lit < 0 && !val)) {
                clause_sat_count[c]++;
                if (clause_sat_count[c] == 1)
                    true_var = v;
                else
                    true_var = -1;
            }
        }

        clause_true_lit[c] = true_var;

        int w = weight(c);

        if (clause_sat_count[c] == 0) {
            addUnsatClause(c);
            for (int lit : formula[c]) {
                int v = (lit > 0 ? lit - 1 : -lit - 1);
                makecount[v] += w;
            }
        }
        else if (clause_sat_count[c] == 1 && true_var != -1) {
            breakcount[true_var] += w;
        }
    }
}

int WalkSAT_MakeCount_PAWS_DET::findUniqueTrueVar(int ci) const {
    int true_var = -1;
    int true_count = 0;

    for (int lit : formula[ci]) {
        int v = (lit > 0 ? lit - 1 : -lit - 1);
        bool val = assignment[v];

        if ((lit > 0 && val) || (lit < 0 && !val)) {
            true_var = v;
            true_count++;

            if (true_count > 1)
                return -1;
        }
    }

    return (true_count == 1) ? true_var : -1;
}

int WalkSAT_MakeCount_PAWS_DET::normalizeWeights()
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


int WalkSAT_MakeCount_PAWS_DET::countWeightedClauses() const
{
    return static_cast<int>(weighted_clauses.size());
}

int WalkSAT_MakeCount_PAWS_DET::maxClauseWeight() const
{
    return current_max_clause_weight;
}

double WalkSAT_MakeCount_PAWS_DET::meanClauseWeight() const
{
    if (tracked_clause_count == 0) {
        return 1.0;
    }

    return static_cast<double>(total_clause_weight) /
           static_cast<double>(tracked_clause_count);
}


bool WalkSAT_MakeCount_PAWS_DET::solve(int max_flips, int max_tries, double p, SolverObserver* observer) {

    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (int t = 0; t < max_tries; ++t) {

        randomAssignment();

        if (observer) {
            observer->onTryStart(t + 1, static_cast<int>(unsat_clauses.size()), unsat_clauses, assignment);
        }

        int prev_unsat_size = unsat_clauses.size();
        int weight_update_events = 0;

        for (int f = 0; f < max_flips; ++f) {

            if (unsat_clauses.empty()) {
                if (observer) {
                    observer->onTryEnd(t + 1, true, 0);
                    observer->onSolveEnd(true, 0);
                }
                return true;
            }

            int c = unsat_clauses[gen() % unsat_clauses.size()];
            const auto& clause = formula[c];

            int flip_var;
            MoveOrigin origin = MoveOrigin::Unknown;

            if (dist(gen) < p) {
                origin = MoveOrigin::Random;
                int lit = clause[gen() % clause.size()];
                flip_var = (lit > 0 ? lit - 1 : -lit - 1);
            } else {
                origin = MoveOrigin::Heuristic;

                int best_var = -1;
                int best_score = INT_MIN;

                for (int lit : clause) {
                    int v = (lit > 0 ? lit - 1 : -lit - 1);

                    int score = makecount[v] - breakcount[v];

                    if (best_var == -1 || score > best_score) {
                        best_score = score;
                        best_var = v;
                    }
                }

                flip_var = best_var;
            }

            const int old_unsat = static_cast<int>(unsat_clauses.size());

            bool old_val = assignment[flip_var];
            assignment[flip_var] = !assignment[flip_var];

            for (const auto& occ : var_occ[flip_var]) {

                int ci = occ.clause;

                bool before = (occ.sign == old_val);
                bool after  = !before;

                int old_count = clause_sat_count[ci];
                int old_true = clause_true_lit[ci];

                if (!before && after)
                    clause_sat_count[ci]++;
                else if (before && !after)
                    clause_sat_count[ci]--;

                int new_count = clause_sat_count[ci];

                int new_true = -1;

                if (new_count == 1) {
                    if (!before && after)
                        new_true = flip_var;
                    else
                        new_true = findUniqueTrueVar(ci);
                }

                clause_true_lit[ci] = new_true;

                int w = weight(ci);

                if (old_count == 1 && old_true != -1)
                    breakcount[old_true] -= w;

                if (new_count == 1 && new_true != -1)
                    breakcount[new_true] += w;

                if (old_count == 0 && new_count > 0) {
                    for (int lit : formula[ci]) {
                        int v = (lit > 0 ? lit - 1 : -lit - 1);
                        makecount[v] -= w;
                    }
                }

                if (old_count > 0 && new_count == 0) {
                    for (int lit : formula[ci]) {
                        int v = (lit > 0 ? lit - 1 : -lit - 1);
                        makecount[v] += w;
                    }
                }

                if (old_count == 0 && new_count > 0)
                    removeUnsatClause(ci);
                else if (old_count > 0 && new_count == 0)
                    addUnsatClause(ci);
            }

            int current_unsat = unsat_clauses.size();

            bool paws_penalty_applied = false;
            int paws_weight_increments = 0;
            bool paws_smoothing_applied = false;
            int paws_weight_decrements = 0;

            // PAWS deterministic weight update:
            // if the move did not improve the number of UNSAT clauses,
            // increase all currently UNSAT clause weights by 1.
            // After maxinc such penalty events, decrement all weights > 1 by 1.
            if (current_unsat >= prev_unsat_size) {
                for (int ci : unsat_clauses) {
                    increaseClauseWeight(ci);
                    ++paws_weight_increments;
                }

                paws_penalty_applied = (paws_weight_increments > 0);

                if (paws_penalty_applied) {
                    ++weight_update_events;

                    if (maxinc > 0 && weight_update_events % maxinc == 0) {
                        paws_weight_decrements = normalizeWeights();
                        paws_smoothing_applied = (paws_weight_decrements > 0);
                    }
                }
            }

            if (observer) {
                observer->onPawsStep(
                    t + 1,
                    f + 1,
                    paws_penalty_applied,
                    paws_weight_increments,
                    paws_smoothing_applied,
                    paws_weight_decrements,
                    countWeightedClauses(),
                    maxClauseWeight(),
                    meanClauseWeight()
                );
            }

            prev_unsat_size = current_unsat;

            if (observer) {
                observer->onFlip(t + 1, flip_var + 1, old_unsat, current_unsat, unsat_clauses, assignment, origin);
            }

            if (unsat_clauses.empty()) {
                if (observer) {
                    observer->onTryEnd(t + 1, true, 0);
                    observer->onSolveEnd(true, 0);
                }
                return true;
            }
        }

        if (observer) {
            observer->onTryEnd(t + 1, false, static_cast<int>(unsat_clauses.size()));
        }
    }

    if (observer) {
        observer->onSolveEnd(false, static_cast<int>(unsat_clauses.size()));
    }
    return false;
}

const std::vector<uint8_t>& WalkSAT_MakeCount_PAWS_DET::getAssignment() const {
    return assignment;
}
