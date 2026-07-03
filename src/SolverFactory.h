#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ISolver.h"
#include "PrecomputedCommunityData.h"

struct SolverParameters {
    int seed = 42;
    double alpha = 1.0;
    double lambda = 1.0;
    double bridge_clause_probability = 0.20;

    // PAWS probabilístico: probabilidad de suavizado por resta.
    // Solo se pasa a las variantes PAWS no deterministas.
    double p_soft = 0.05;

    // PAWS_DET: suavizado determinista por resta cada maxinc eventos.
    int maxinc = 20;
};

class SolverFactory {
public:
    static std::unique_ptr<ISolver> create(
        const std::string& solver_name,
        const std::vector<std::vector<int>>& formula,
        int num_variables,
        const SolverParameters& params = {},
        const PrecomputedCommunityData* precomputed = nullptr

    );
};
