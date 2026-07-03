#ifndef INSTANCE_PROCESSING_H
#define INSTANCE_PROCESSING_H

#include <vector>
#include <string>
#include <unordered_map>

class InstanceProcessing {
public:

    enum ClauseType {
        INTRA_COMMUNITY = 0,
        BRIDGE = 1,
        HIGHLY_MIXED = 2
    };

    using WeightedGraph =
        std::vector<std::unordered_map<int, double>>;

    /*
        Build weighted VIG from parsed CNF
    */
    static WeightedGraph buildWeightedVIG(
        const std::vector<std::vector<int>>& formula,
        int numVariables,
        bool normalizeByClauseSize = true
    );

    /*
        Louvain (igraph multilevel modularity)
        returns variable -> community
    */
    static std::vector<int> runLouvain(
        const WeightedGraph& graph
    );

    /*
        Clause structural classification
    */
    static std::vector<ClauseType> characterizeClauses(
        const std::vector<std::vector<int>>& formula,
        const std::vector<int>& varToCommunity
    );
};

#endif
