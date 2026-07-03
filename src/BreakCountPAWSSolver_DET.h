#pragma once

#include "ISolver.h"
#include "WalkSAT_BreakCount_PAWS_DET.h"

#include <cstdint>
#include <vector>

class BreakCountPAWSSolver_DET : public ISolver {
public:
    BreakCountPAWSSolver_DET(
        const std::vector<std::vector<int>>& formula,
        int num_variables,
        int seed = 0,
        int maxinc = 20
    )
        : solver(formula, num_variables, seed, maxinc)
    {
    }

    bool solve(
        int max_flips,
        int max_tries,
        double p,
        SolverObserver* observer = nullptr
    ) override
    {
        return solver.solve(max_flips, max_tries, p, observer);
    }

    const std::vector<uint8_t>& getAssignment() const override
    {
        const auto& raw = solver.getAssignment();

        assignment_buffer.clear();
        assignment_buffer.reserve(raw.size());

        for (auto value : raw) {
            assignment_buffer.push_back(static_cast<uint8_t>(value));
        }

        return assignment_buffer;
    }

private:
    WalkSAT_BreakCount_PAWS_DET solver;
    mutable std::vector<uint8_t> assignment_buffer;
};
