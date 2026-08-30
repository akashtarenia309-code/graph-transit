#pragma once

#include "Graph.hpp"

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
    // (boarding a new bus).
    //
    // This lets us find shortest paths in O(V + E) using a double-ended
    // queue instead of a full Dijkstra algorithm:
    //   - weight-0 edges are pushed to the front,
    //   - weight-1 edges are pushed to the back.
    //
    // The returned cost between two stop nodes is exactly the minimum
    // number of buses that must be boarded.
    //
    // Returns -1 if destination is unreachable from source.
    static int zeroOneBFS(
        const Graph& graph,
        int source,
        int destination
    );
};