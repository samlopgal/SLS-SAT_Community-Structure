// PrecomputedCommunityBuilder.cpp

#include "PrecomputedCommunityBuilder.h"

#include <unordered_set>
#include <cmath>

PrecomputedCommunityData
PrecomputedCommunityBuilder::build(
    const std::vector<std::vector<int>>& formula,
    const std::vector<int>& var_to_community
)
{
    PrecomputedCommunityData data;

    data.var_to_community = var_to_community;

    // ==========================================
    // MAX COMMUNITY ID
    // ==========================================

    int max_comm = 0;

    for (int v = 1; v < var_to_community.size(); ++v)
        max_comm =
            std::max(
                max_comm,
                var_to_community[v]
            );

    // ==========================================
    // COMMUNITY SIZE
    // ==========================================

    data.community_size.assign(
        max_comm + 1,
        0
    );

    for (int v = 1; v < var_to_community.size(); ++v)
        data.community_size[
            var_to_community[v]
        ]++;

    data.total_community_size = 0.0;

    for (int s : data.community_size)
        data.total_community_size += s;

    // ==========================================
    // CLAUSE COMMUNITIES
    // ==========================================

    data.clause_communities.resize(
        formula.size()
    );

    data.community_total.assign(
        max_comm + 1,
        0
    );

    for (int ci = 0; ci < formula.size(); ++ci) {

        std::unordered_set<int> comms;

        for (int lit : formula[ci]) {

            int v = std::abs(lit);

            comms.insert(
                var_to_community[v]
            );
        }

        data.clause_communities[ci] =
            std::vector<int>(
                comms.begin(),
                comms.end()
            );

        for (int c :
             data.clause_communities[ci])
        {
            data.community_total[c]++;
        }
    }

    return data;
}
