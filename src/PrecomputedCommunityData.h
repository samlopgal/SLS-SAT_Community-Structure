// PrecomputedCommunityData.h

#pragma once

#include <vector>

struct PrecomputedCommunityData {

    // ==========================================
    // Variable -> community
    // ==========================================

    std::vector<int> var_to_community;

    // ==========================================
    // Clause -> involved communities
    // ==========================================

    std::vector<std::vector<int>> clause_communities;

    // ==========================================
    // Community statistics
    // ==========================================

    std::vector<int> community_total;

    std::vector<int> community_size;

    double total_community_size = 0.0;
};
