// PrecomputedCommunityBuilder.h

#pragma once

#include "PrecomputedCommunityData.h"

#include <vector>

class PrecomputedCommunityBuilder {
public:

    static PrecomputedCommunityData build(
        const std::vector<std::vector<int>>& formula,
        const std::vector<int>& var_to_community
    );
};
