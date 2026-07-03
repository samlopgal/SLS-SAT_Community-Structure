#pragma once

#include "ISolver.h"
#include "WalkSAT_BreakCount.h"

class BreakCountSolver : public ISolver {
public:
    BreakCountSolver(
        const std::vector<std::vector<int>>& formula,
        int num_variables,
        int seed = 0
    )
        : solver(formula, num_variables, seed)
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
        return solver.getAssignment();
    }

private:
    WalkSAT_BreakCount solver;
};
