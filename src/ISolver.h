#pragma once

#include <cstdint>
#include <vector>

#include "SolverObserver.h"

class ISolver {
public:
    virtual ~ISolver() = default;

    virtual bool solve(
        int max_flips,
        int max_tries,
        double p,
        SolverObserver* observer = nullptr
    ) = 0;

    virtual const std::vector<uint8_t>& getAssignment() const = 0;
};
