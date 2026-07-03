#pragma once

#include <cstdint>
#include <vector>

class SolutionVerifier {
public:
    // Devuelve true si assignment satisface formula.
    // Acepta assignment 0-based de tamaño num_variables o 1-based de tamaño num_variables + 1.
    static bool verify(
        const std::vector<std::vector<int>>& formula,
        int num_variables,
        const std::vector<uint8_t>& assignment
    );
};
