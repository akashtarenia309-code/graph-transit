#pragma once

#include <vector>

// FlowNetwork models passenger-flow-style problems: given capacities
// on connections (e.g. how many passengers per unit time a route
// segment can carry), find the maximum flow from a source to a sink.
//
// This is intentionally a separate class from Graph rather than built
// on top of it: flow algorithms need mutable per-edge state (residual
// capacity, current flow) that Graph's fixed-weight Edge struct does
// not represent, and pairing every edge with its reverse residual
// edge is specific to flow problems.
//
// Implements Dinic's algorithm:
//   1. Repeatedly build a "level graph" via BFS from the source,
//      recording each node's distance (level) from the source using
//      only edges with remaining residual capacity.
//   2. If the sink is unreachable in the level graph, no more
//      augmenting paths exist — stop.
//   3. Otherwise, use DFS restricted to the level graph (only moving
//      from a lower level to the next higher level) to push as much
//      "blocking flow" as possible along multiple augmenting paths.
//   4. Repeat from step 1.
//
// Complexity: O(V^2 * E) in general graphs, where V is the number of
// nodes and E is the number of edges (this bound improves to
// O(E * sqrt(V)) for unit-capacity graphs, a common case for simple
// flow-capacity transportation models).
class FlowNetwork {
public:
    // Constructs a flow network with the given number of nodes
    // (IDs 0 to numNodes - 1) and no edges.
    explicit FlowNetwork(int numNodes);

    // Adds a directed edge from -> to with the given capacity. A
    // reverse residual edge with capacity 0 is created internally to
    // support flow cancellation.
    void addEdge(int from, int to, long long capacity);

    // Computes and returns the maximum flow from source to sink.
    long long maxFlow(int source, int sink);

private:
    // A directed edge in the residual graph: how much more flow can
    // currently be pushed from this edge's implicit source to `to`.
    struct FlowEdge {
        int to;
        long long capacity;
        long long flow;
    };

    int numNodes_;

    // All edges (forward and reverse residual), stored flat so that
    // an edge and its reverse can be found via XOR-paired indices.
    std::vector<FlowEdge> edges_;

    // adjacency_[node] holds indices into edges_ for node's outgoing
    // edges (including residual reverse edges).
    std::vector<std::vector<int>> adjacency_;

    // Rebuilds the level graph via BFS from source using only edges
    // with remaining residual capacity. Returns false if sink is
    // unreachable (meaning maxFlow is complete).
    bool buildLevelGraph(int source, int sink, std::vector<int>& level) const;

    // Pushes blocking flow through the current level graph via DFS,
    // using `iter` to skip edges already fully explored from each
    // node (Dinic's standard "current arc" optimization).
    long long sendBlockingFlow(
        int node,
        int sink,
        long long pushed,
        std::vector<int>& level,
        std::vector<int>& iter
    );
};