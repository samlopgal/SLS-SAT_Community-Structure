#include "WalkSAT_BreakCount.h"
#include <cmath>
#include <climits>

// Constructor
WalkSAT_BreakCount::WalkSAT_BreakCount(const std::vector<std::vector<int>>& formula_, int num_variables_, int seed_)
    : formula(formula_), num_variables(num_variables_), gen(seed_)
{
    int num_clauses = formula.size();

    assignment.resize(num_variables);
    clause_sat_count.resize(num_clauses);
    clause_true_lit.resize(num_clauses);
    clause_pos_in_unsat.resize(num_clauses);
    breakcount.resize(num_variables);

    var_occ.resize(num_variables);
    unsat_clauses.reserve(num_clauses);

    // Construir estructuras de ocurrencias por variable
    for (int i = 0; i < num_clauses; ++i) {
        for (int lit : formula[i]) {
            int var = (lit > 0) ? (lit - 1) : (-lit - 1);

            Occurrence occ;
            occ.clause = i;
            occ.sign = (lit > 0);

            var_occ[var].push_back(occ);
        }
    }
}

void WalkSAT_BreakCount::addUnsatClause(int ci) {
    clause_pos_in_unsat[ci] = unsat_clauses.size();
    unsat_clauses.push_back(ci);
}

void WalkSAT_BreakCount::removeUnsatClause(int ci) {
    int pos = clause_pos_in_unsat[ci];
    int last = unsat_clauses.back();

    unsat_clauses[pos] = last;
    clause_pos_in_unsat[last] = pos;

    unsat_clauses.pop_back();
    clause_pos_in_unsat[ci] = -1;
}

// Asignación aleatoria
void WalkSAT_BreakCount::randomAssignment() {
    for (int i = 0; i < num_variables; ++i)
        assignment[i] = (gen() & 1);

    initializeClauseData();
}

// Inicializar información de cláusulas y breakcount
void WalkSAT_BreakCount::initializeClauseData() {
    int num_clauses = formula.size();

    std::fill(clause_sat_count.begin(), clause_sat_count.end(), 0);
    std::fill(clause_true_lit.begin(), clause_true_lit.end(), -1);
    std::fill(clause_pos_in_unsat.begin(), clause_pos_in_unsat.end(), -1);
    std::fill(breakcount.begin(), breakcount.end(), 0);

    unsat_clauses.clear();

    for (int i = 0; i < num_clauses; ++i) {
        int true_var = -1;
        for (int lit : formula[i]) {
            int var = (lit > 0) ? (lit - 1) : (-lit - 1);
            bool val = assignment[var];

            if ((lit > 0 && val) || (lit < 0 && !val)) {
                clause_sat_count[i]++;
                if (clause_sat_count[i] == 1)
                    true_var = var;
                else
                    true_var = -1;
            }
        }
        clause_true_lit[i] = true_var;

        if (clause_sat_count[i] == 0)
            addUnsatClause(i);

        if (clause_sat_count[i] == 1 && true_var != -1)
            breakcount[true_var]++;
    }
}

int WalkSAT_BreakCount::findUniqueTrueVar(int ci) const {
    int true_var = -1;
    int true_count = 0;

    for (int lit : formula[ci]) {
        int var = (lit > 0) ? (lit - 1) : (-lit - 1);
        bool val = assignment[var];

        if ((lit > 0 && val) || (lit < 0 && !val)) {
            true_var = var;
            true_count++;

            if (true_count > 1)
                return -1;
        }
    }

    return (true_count == 1) ? true_var : -1;
}

// WalkSAT con propagación incremental de cambios
bool WalkSAT_BreakCount::solve(int max_flips, int max_tries, double p, SolverObserver* observer) {

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

            int c_idx = unsat_clauses[gen() % unsat_clauses.size()];
            const auto& clause = formula[c_idx];

            int flip_var;
            MoveOrigin origin = MoveOrigin::Unknown;
            double r = (double)(gen() & 0xFFFF) / 65535.0;

            if (r < p) {
                origin = MoveOrigin::Random;
                int lit = clause[gen() % clause.size()];
                flip_var = (lit > 0) ? (lit - 1) : (-lit - 1);
            } else {
                origin = MoveOrigin::Heuristic;
                int lit0 = clause[0];
                int best_var = (lit0 > 0) ? (lit0 - 1) : (-lit0 - 1);
                int best_break = breakcount[best_var];

                for (int lit : clause) {
                    int var = (lit > 0) ? (lit - 1) : (-lit - 1);

                    if (breakcount[var] == 0) {
                        best_var = var;
                        break;
                    }
                    if (breakcount[var] < best_break) {
                        best_break = breakcount[var];
                        best_var = var;
                    }
                }
                flip_var = best_var;
            }

            const int old_unsat = static_cast<int>(unsat_clauses.size());

            // Aplicar flip
            bool old_val = assignment[flip_var];
            assignment[flip_var] = !assignment[flip_var];

            // Propagación incremental de cambios
            for (const Occurrence& occ : var_occ[flip_var]) {
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

                if (old_count == 1 && old_true != -1)
                    breakcount[old_true]--;

                if (new_count == 1 && new_true != -1)
                    breakcount[new_true]++;

                if (old_count == 0 && new_count > 0)
                    removeUnsatClause(ci);
                else if (old_count > 0 && new_count == 0)
                    addUnsatClause(ci);
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

const std::vector<uint8_t>& WalkSAT_BreakCount::getAssignment() const {
    return assignment;
}
