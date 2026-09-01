#pragma once

#include "Graph.hpp"

#include <vector>

// Result of a shortest-path query: the total cost and the actual
// sequence of graph node IDs from source to destination (inclusive).
//
// If the destination is unreachable, cost is -1 and path is empty.
struct PathResult {
    int cost;
    std::vector<int> path;
};

// Algorithms holds the graph algorithms used to solve transportation
// routing problems on top of a Graph produced by GraphTransformer.
//
// Like GraphTransformer, this class carries no state of its own — it
// is purely a namespace-like grouping of static algorithm functions
// that operate on a given Graph.
class Algorithms {
public:
    // Computes the minimum-cost path from source to destination using
    // 0-1 BFS.
    //
    // 0-1 BFS is appropriate here because, after the dummy-bus-node
    // transformation, every edge in the graph has weight either 0
    // (staying on the same bus, moving between its stops) or 1
    // (boarding a new bus). This lets us find shortest paths in O(V + E)
    // using a double-ended queue instead of a full Dijkstra:
    //   - weight-0 edges are conceptually pushed to the FRONT of the
    //     deque (processed next, since they don't increase cost),
    //   - weight-1 edges are conceptually pushed to the BACK of the
    //     deque (processed later, since they do increase cost).
    //
    // The returned cost from a source stop to a destination stop is
    // exactly the minimum number of buses that must be boarded.
    //
    // Returns -1 if destination is unreachable from source.
    static int zeroOneBFS(
        const Graph& graph,
        int source,
        int destination
    );

    // Same algorithm as zeroOneBFS, but also reconstructs and returns
    // the actual sequence of graph nodes (stops and dummy bus nodes)
    // that make up a minimum-cost path from source to destination.
    //
    // This is useful for turning a bus-boarding count into a concrete,
    // human-readable itinerary: which stop to board at, which dummy
    // bus node represents riding a given bus, and where to transfer.
    //
    // Returns PathResult{-1, {}} if destination is unreachable.
    static PathResult zeroOneBFSWithPath(
        const Graph& graph,
        int source,
        int destination
    );

    // Computes the minimum-cost path from source to destination using
    // Dijkstra's algorithm with a binary heap (std::priority_queue).
    //
    // Unlike zeroOneBFS, this supports arbitrary non-negative edge
    // weights (e.g. real travel times in a "fastest route" graph, not
    // just the 0/1 dummy-bus-node weights). Used for problems where
    // edges carry general costs rather than pure boarding counts.
    //
    // Complexity: O((V + E) log V), where V is the number of nodes and
    // E is the number of edges, due to each edge relaxation performing
    // at most one O(log V) priority_queue push.
    //
    // Returns PathResult{-1, {}} if destination is unreachable.
    // Throws std::invalid_argument if any edge has a negative weight.
    static PathResult dijkstra(
        const Graph& graph,
        int source,
        int destination
    );

private:
    // Shared 0-1 BFS traversal used by both zeroOneBFS and
    // zeroOneBFSWithPath. Performs all validation, runs the traversal,
    // and returns the final distance and parent vectors so callers can
    // extract just the cost or reconstruct the full path as needed.
    static void runZeroOneBFS(
        const Graph& graph,
        int source,
        int destination,
        std::vector<int>& distance,
        std::vector<int>& parent
    );
};