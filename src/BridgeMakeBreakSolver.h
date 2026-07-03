

#pragma once

#include "ISolver.h"
#include "WalkSAT_Bridge_MakeBreak.h"
#include "PrecomputedCommunityData.h"

class BridgeMakeBreakSolver : public ISolver {
public:

    BridgeMakeBreakSolver(
        const std::vector<std::vector<int>>& formula,
        int num_variables,
        int seed,
        const PrecomputedCommunityData* precomputed
    )
        : solver(
              formula,
              num_variables,
              seed,
              precomputed
          )
    {
    }

    bool solve(
        int max_flips,
        int max_tries,
        double p,
        SolverObserver* observer = nullptr
    ) override
    {
        return solver.solve(
            max_flips,
            max_tries,
            p,
            observer
        );
    }

    const std::vector<uint8_t>&
    getAssignment() const override
    {
        return solver.getAssignment();
    }

private:
    WalkSAT_Bridge_MakeBreak solver;
};
