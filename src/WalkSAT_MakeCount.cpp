#include "WalkSAT_MakeCount.h"
#include <algorithm>
#include <cmath>
#include <climits>

// Constructor
WalkSAT_MakeCount::WalkSAT_MakeCount(
    const std::vector<std::vector<int>>& formula_,
    int num_variables_,
    int seed_)
    : formula(formula_), num_variables(num_variables_), gen(seed_)
{
    int n = formula.size();

    assignment.resize(num_variables);
    clause_sat_count.resize(n);
    clause_true_lit.resize(n);
    clause_pos_in_unsat.resize(n);

    breakcount.resize(num_variables);
    makecount.resize(num_variables);

    var_occ.resize(num_variables);

    unsat_clauses.reserve(n);

    // Construcción de ocurrencias variable → cláusulas
    for (int i = 0; i < n; ++i) {
        for (int lit : formula[i]) {
            int v = (lit > 0) ? (lit - 1) : (-lit - 1);
            var_occ[v].push_back({i, lit > 0});
        }
    }
}

// ----------------------------

void WalkSAT_MakeCount::addUnsatClause(int ci) {
    clause_pos_in_unsat[ci] = unsat_clauses.size();
    unsat_clauses.push_back(ci);
}

// ----------------------------

void WalkSAT_MakeCount::removeUnsatClause(int ci) {
    int pos = clause_pos_in_unsat[ci];
    int last = unsat_clauses.back();

    unsat_clauses[pos] = last;
    clause_pos_in_unsat[last] = pos;

    unsat_clauses.pop_back();
    clause_pos_in_unsat[ci] = -1;
}

// ----------------------------

void WalkSAT_MakeCount::randomAssignment() {
    for (int i = 0; i < num_variables; ++i)
        assignment[i] = gen() & 1;

    initializeClauseData();
}

// ----------------------------

void WalkSAT_MakeCount::initializeClauseData() {
    int n = formula.size();

    std::fill(clause_sat_count.begin(), clause_sat_count.end(), 0);
    std::fill(clause_true_lit.begin(), clause_true_lit.end(), -1);
    std::fill(clause_pos_in_unsat.begin(), clause_pos_in_unsat.end(), -1);
    std::fill(breakcount.begin(), breakcount.end(), 0);
    std::fill(makecount.begin(), makecount.end(), 0);

    unsat_clauses.clear();

    for (int i = 0; i < n; ++i) {

        int sat = 0;
        int first_true_var = -1;

        for (int lit : formula[i]) {
            int v = (lit > 0) ? (lit - 1) : (-lit - 1);
            bool val = (lit > 0) ? assignment[v] : !assignment[v];

            if (val) {
                sat++;
                if (sat == 1)
                    first_true_var = v;
                else
                    first_true_var = -1;
            }
        }

        clause_sat_count[i] = sat;
        clause_true_lit[i] = first_true_var;

        if (sat == 0) {
            addUnsatClause(i);

            for (int lit : formula[i]) {
                int v = (lit > 0) ? (lit - 1) : (-lit - 1);
                makecount[v]++;
            }
        }

        if (sat == 1 && first_true_var != -1)
            breakcount[first_true_var]++;
    }
}

// ----------------------------

int WalkSAT_MakeCount::findUniqueTrueVar(int ci) const {
    int true_var = -1;
    int true_count = 0;

    for (int lit : formula[ci]) {
        int v = (lit > 0) ? (lit - 1) : (-lit - 1);
        bool val = (lit > 0) ? assignment[v] : !assignment[v];

        if (val) {
            true_var = v;
            true_count++;

            if (true_count > 1)
                return -1;
        }
    }

    return (true_count == 1) ? true_var : -1;
}

// ----------------------------

bool WalkSAT_MakeCount::solve(int max_flips, int max_tries, double p, SolverObserver* observer) {

    for (int t = 0; t < max_tries; ++t) {
        randomAssignment();

        if (observer) {
            observer->onTryStart(t + 1, static_cast<int>(unsat_clauses.size()), unsat_clauses, assignment);
        }

        for (int f = 0; f < max_flips; ++f) {

            if (unsat_clauses.empty()) {
                if (observer) {
                    observer->onTryEnd(t + 1, true, 0);
                    observer->onSolveEnd(true, 0);
                }
                return true;
            }

            int ci = unsat_clauses[gen() % unsat_clauses.size()];

            const auto& clause = formula[ci];

            int flip_var;
            MoveOrigin origin = MoveOrigin::Unknown;

            if ((double)(gen() & 0xFFFF) / 65535.0 < p) {
                origin = MoveOrigin::Random;
                int lit = clause[gen() % clause.size()];
                flip_var = (lit > 0) ? (lit - 1) : (-lit - 1);
            } else {
                origin = MoveOrigin::Heuristic;
                int best = (clause[0] > 0) ? (clause[0] - 1) : (-clause[0] - 1);
                int best_score = makecount[best] - breakcount[best];

                for (int lit : clause) {
                    int v = (lit > 0) ? (lit - 1) : (-lit - 1);
                    int score = makecount[v] - breakcount[v];

                    if (score > best_score) {
                        best_score = score;
                        best = v;
                    }
                }

                flip_var = best;
            }

            const int old_unsat = static_cast<int>(unsat_clauses.size());

            bool old_val = assignment[flip_var];
            assignment[flip_var] = !old_val;

            // ACTUALIZACIÓN COMPLETAMENTE EVENT-DRIVEN
            for (const Occurrence& occ : var_occ[flip_var]) {

                int ci2 = occ.clause;

                bool before = (occ.sign == old_val);
                bool after  = !before;

                int old_sat = clause_sat_count[ci2];
                int old_true = clause_true_lit[ci2];

                if (!before && after)
                    clause_sat_count[ci2]++;
                else if (before && !after)
                    clause_sat_count[ci2]--;

                int new_sat = clause_sat_count[ci2];

                int new_true = -1;

                if (new_sat == 1) {
                    if (!before && after)
                        new_true = flip_var;
                    else
                        new_true = findUniqueTrueVar(ci2);
                }

                clause_true_lit[ci2] = new_true;

                // transición a satisfacible
                if (old_sat == 0 && new_sat > 0) {
                    removeUnsatClause(ci2);

                    for (int lit : formula[ci2]) {
                        int v = (lit > 0) ? (lit - 1) : (-lit - 1);
                        makecount[v]--;
                    }
                }

                // transición a insatisfacible
                if (old_sat > 0 && new_sat == 0) {
                    addUnsatClause(ci2);

                    for (int lit : formula[ci2]) {
                        int v = (lit > 0) ? (lit - 1) : (-lit - 1);
                        makecount[v]++;
                    }
                }

                // breakcount incremental
                if (old_sat == 1 && old_true != -1)
                    breakcount[old_true]--;

                if (new_sat == 1 && new_true != -1)
                    breakcount[new_true]++;
            }

            const int new_unsat = static_cast<int>(unsat_clauses.size());
            if (observer) {
                observer->onFlip(t + 1, flip_var + 1, old_unsat, new_unsat, unsat_clauses, assignment, origin);
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

const std::vector<uint8_t>& WalkSAT_MakeCount::getAssignment() const {
    return assignment;
}
