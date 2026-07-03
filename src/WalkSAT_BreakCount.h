#ifndef WALKSAT_BREAKCOUNT_H
#define WALKSAT_BREAKCOUNT_H

#include <vector>
#include <algorithm>
#include <random>
#include "Ocurrence.h"
#include "SolverObserver.h"

class WalkSAT_BreakCount {
public:
    WalkSAT_BreakCount(const std::vector<std::vector<int>>& formula_, int num_variables_, int seed_);

    bool solve(int max_flips, int max_tries, double p, SolverObserver* observer = nullptr);
    const std::vector<uint8_t>& getAssignment() const;

private:

    std::vector<std::vector<int>> formula;
    int num_variables;

    std::vector<uint8_t> assignment;

    std::vector<int> clause_sat_count;
    std::vector<int> clause_true_lit;
    std::vector<int> unsat_clauses;
    std::vector<int> clause_pos_in_unsat;
    std::vector<std::vector<Occurrence>> var_occ;
    std::vector<int> breakcount;

    std::mt19937 gen;

    void randomAssignment();
    void initializeClauseData();

    void addUnsatClause(int ci);
    void removeUnsatClause(int ci);

    int findUniqueTrueVar(int ci) const;
};

#endif
