#include "InstanceProcessing.h"

#include <stdexcept>
#include <cmath>
#include <set>
#include <unordered_set>

#include <igraph/igraph.h>



// ======================================================
// BUILD WEIGHTED VIG
// ======================================================

InstanceProcessing::WeightedGraph
InstanceProcessing::buildWeightedVIG(
    const std::vector<std::vector<int>>& formula,
    int numVariables,
    bool normalizeByClauseSize
) {

    WeightedGraph graph(numVariables + 1);

    for (const auto& clause : formula) {

        int k = clause.size();
        if (k < 2) continue;

        double inc = 1.0;

        if (normalizeByClauseSize) {
            inc = 1.0 / ((k * (k - 1)) / 2.0);
        }

        for (int i = 0; i < k; ++i) {
            for (int j = i + 1; j < k; ++j) {

                int u = std::abs(clause[i]);
                int v = std::abs(clause[j]);

                graph[u][v] += inc;
                graph[v][u] += inc;
            }
        }
    }

    return graph;
}



// ======================================================
// LOUVAIN (FIXED FOR YOUR IGRAPH VERSION)
// ======================================================

std::vector<int>
InstanceProcessing::runLouvain(
    const WeightedGraph& graph
) {

    int n = graph.size() - 1;

    igraph_t g;

    igraph_vector_int_t edges;
    igraph_vector_t weights;

    igraph_vector_int_init(&edges, 0);
    igraph_vector_init(&weights, 0);

    std::set<std::pair<int,int>> added;

    for (int u = 1; u <= n; ++u) {

        for (const auto& [v, w] : graph[u]) {

            int a = std::min(u, v);
            int b = std::max(u, v);

            if (added.count({a,b})) continue;

            added.insert({a,b});

            igraph_vector_int_push_back(&edges, a - 1);
            igraph_vector_int_push_back(&edges, b - 1);

            igraph_vector_push_back(&weights, w);
        }
    }

    igraph_create(&g, &edges, n, IGRAPH_UNDIRECTED);

    igraph_vector_int_t membership;
    igraph_vector_int_init(&membership, 0);

    igraph_matrix_int_t merges;
    igraph_matrix_int_init(&merges, 0, 0);   // ✅ FIX IMPORTANTE

    igraph_vector_t modularity;
    igraph_vector_init(&modularity, 0);

    // ================================
    // CORRECT CALL FOR YOUR IGRAPH
    // ================================
    igraph_real_t resolution = 1.0;

    igraph_community_multilevel(
        &g,
        &weights,
        resolution,
        &membership,
        &merges,
        &modularity
    );

    std::vector<int> communities(n + 1);

    for (int i = 0; i < n; ++i) {
        communities[i + 1] = VECTOR(membership)[i];
    }

    igraph_vector_int_destroy(&membership);
    igraph_matrix_int_destroy(&merges);
    igraph_vector_destroy(&modularity);

    igraph_vector_destroy(&weights);
    igraph_vector_int_destroy(&edges);
    igraph_destroy(&g);

    return communities;
}



// ======================================================
// CHARACTERIZE CLAUSES
// ======================================================

std::vector<InstanceProcessing::ClauseType>
InstanceProcessing::characterizeClauses(
    const std::vector<std::vector<int>>& formula,
    const std::vector<int>& varToCommunity
) {

    std::vector<ClauseType> result;
    result.reserve(formula.size());

    for (const auto& clause : formula) {

        std::unordered_set<int> comms;

        for (int lit : clause) {

            int v = std::abs(lit);

            if (v < (int)varToCommunity.size()) {
                comms.insert(varToCommunity[v]);
            }
        }

        if (comms.size() == 1) {
            result.push_back(INTRA_COMMUNITY);
        }
        else if (comms.size() == 2) {
            result.push_back(BRIDGE);
        }
        else {
            result.push_back(HIGHLY_MIXED);
        }
    }

    return result;
}
