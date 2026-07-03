#pragma once

#include <vector>
#include <random>
#include <cstdint>

#include "Ocurrence.h"
#include "SolverObserver.h"

class WalkSAT_MakeCount_PAWS_DET {
public:
    WalkSAT_MakeCount_PAWS_DET(
        const std::vector<std::vector<int>>& formula_,
        int num_variables_,
        int seed_ = 0,
        int maxinc_ = 20
    );

    bool solve(
        int max_flips,
        int max_tries,
        double p,
        SolverObserver* observer = nullptr
    );

    const std::vector<uint8_t>& getAssignment() const;

private:
    const std::vector<std::vector<int>>& formula;
    int num_variables;

    std::vector<uint8_t> assignment;

    std::vector<int> clause_sat_count;
    std::vector<int> clause_true_lit;

    std::vector<int> clause_weight;

    // Incremental PAWS weight statistics.
    // These make maxClauseWeight() and meanClauseWeight() O(1)
    // instead of scanning all clauses on every flip.
    long long total_clause_weight = 0;
    int tracked_clause_count = 0;
    int current_max_clause_weight = 1;
    std::vector<int> weight_frequency;

    std::vector<std::vector<Occurrence>> var_occ;

    std::vector<int> unsat_clauses;
    std::vector<int> clause_pos_in_unsat;

    std::vector<int> weighted_clauses;
    std::vector<int> clause_pos_in_weighted;

    std::vector<int> breakcount;
    std::vector<int> makecount;

    std::mt19937 gen;

    // PAWS determinista: tras maxinc eventos de penalización,
    // todos los pesos mayores que 1 se decrementan en una unidad.
    // Si maxinc <= 0, el suavizado queda desactivado.
    int maxinc;

private:
    void randomAssignment();
    void initializeClauseData();

    void addUnsatClause(int ci);
    void removeUnsatClause(int ci);

    void addWeightedClause(int ci);
    void removeWeightedClause(int ci);

    void resetWeightStatistics(int tracked_count);
    void noteWeightIncrease(int old_weight);
    void noteWeightDecrease(int old_weight);

    void increaseClauseWeight(int ci);
    void decreaseClauseWeight(int ci);
    int normalizeWeights();

    int countWeightedClauses() const;
    int maxClauseWeight() const;
    double meanClauseWeight() const;

    int findUniqueTrueVar(int ci) const;

    inline int weight(int ci) const;
};
