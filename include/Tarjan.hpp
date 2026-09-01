#pragma once

#include "Graph.hpp"

#include <utility>
#include <vector>

// Result of running Tarjan's critical-infrastructure analysis on a
// transportation network.
//
// - articulationPoints: stop node IDs whose removal (along with their
//   incident connections) would disconnect the network into more
//   pieces than it currently has. These are "critical stops" — losing
//   one of them cuts off part of the network.
// - bridges: individual (u, v) connections whose removal alone would
//   disconnect the network. These are "critical connections" — routes
//   with no redundant alternative.
// - connectedComponents: the network's connected components, useful
//   for confirming whether the network is fully connected to begin
//   with before critical points/edges are even meaningful.
struct TarjanResult {
    std::vector<int> articulationPoints;
    std::vector<std::pair<int, int>> bridges;
    std::vector<std::vector<int>> connectedComponents;
};

// Tarjan performs critical-infrastructure analysis on a transportation
// network modeled as an UNDIRECTED graph of direct stop-to-stop
// connections (built with Graph::addUndirectedEdge) — for example, an
// edge between two stops that are consecutive on some bus route.
//
// This is deliberately a different graph than the dummy-bus-node graph
// produced by GraphTransformer: articulation points and bridges are an
// undirected-connectivity concept, answering "which stops or direct
// connections, if lost, would disconnect the network?" — not "how many
// buses must be boarded?".
//
// Like Algorithms and GraphTransformer, this class carries no state; it
// is a stateless grouping of static analysis functions.
class Tarjan {
public:
    // Runs a single DFS-based pass computing articulation points,
    // bridges, and connected components for the given undirected graph.
    //
    // Complexity: O(V + E), where V is the number of nodes and E is the
    // number of edges, since each node and edge is visited a constant
    // number of times during the DFS.
    //
    // Assumes the graph has no duplicate parallel edges between the
    // same pair of nodes (a simple graph). Node IDs are validated
    // implicitly via Graph's own bounds.
    static TarjanResult analyze(const Graph& graph);

private:
    // Recursive DFS helper computing discovery times, low-link values,
    // articulation points, and bridges rooted at `node`.
    static void dfs(
        const Graph& graph,
        int node,
        int parent,
        int& timer,
        std::vector<int>& discoveryTime,
        std::vector<int>& lowLink,
        std::vector<bool>& visited,
        std::vector<bool>& isArticulationPoint,
        std::vector<std::pair<int, int>>& bridges,
        std::vector<int>& currentComponent
    );
};