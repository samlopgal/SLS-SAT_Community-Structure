// PrecomputedCommunityIO.h

#pragma once

#include "PrecomputedCommunityData.h"

#include <string>

class PrecomputedCommunityIO {
public:

    static void save(
        const std::string& path,
        const PrecomputedCommunityData& data
    );

    static PrecomputedCommunityData load(
        const std::string& path
    );
};
