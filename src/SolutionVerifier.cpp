#include "SolutionVerifier.h"

#include <cstdlib>

bool SolutionVerifier::verify(
    const std::vector<std::vector<int>>& formula,
    int num_variables,
    const std::vector<uint8_t>& assignment)
{
    const bool one_based =
        assignment.size() == static_cast<std::size_t>(num_variables + 1);

    const bool zero_based =
        assignment.size() == static_cast<std::size_t>(num_variables);

    if (!one_based && !zero_based) {
        return false;
    }

    for (const auto& clause : formula) {
        bool clause_satisfied = false;

        for (int lit : clause) {
            const int var = std::abs(lit);
            if (var < 1 || var > num_variables) {
                return false;
            }

            const int idx = one_based ? var : var - 1;
            const bool value = assignment[idx] != 0;

            if ((lit > 0 && value) || (lit < 0 && !value)) {
                clause_satisfied = true;
                break;
            }
        }

        if (!clause_satisfied) {
            return false;
        }
    }

    return true;
}
