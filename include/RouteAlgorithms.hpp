#pragma once

#include "Algorithms.hpp"
#include "Graph.hpp"

#include <vector>

// RouteAlgorithms holds higher-level routing techniques built on top
// of Algorithms' single-path primitives (zeroOneBFS, dijkstra):
//
//   1. Yen's algorithm for K shortest *simple* (loopless) paths,
//      giving passengers alternative routes beyond just the single
//      fastest one.
//
//   2. A layered / state-space graph transformation for answering
//      "can I reach the destination using at most K transfers?" —
//      modeling the state (stop, number of buses boarded so far)
//      explicitly, rather than only computing the single minimum.
//
// Like the other algorithm classes in this project, RouteAlgorithms
// is stateless — a namespace-like grouping of static functions.
class RouteAlgorithms {
public:
    // Returns up to k shortest simple paths from source to destination,
    // in increasing order of total cost, using Yen's algorithm with
    // Dijkstra as the underlying shortest-path subroutine.
    //
    // Yen's algorithm works by treating the current shortest known path
    // as the baseline, then for each node along it ("the spur node"),
    // temporarily removing edges and nodes that would recreate
    // previously found paths, and re-running Dijkstra from the spur
    // node to find an alternative ("spur path"). The best candidate
    // across all spur nodes becomes the next shortest path.
    //
    // If fewer than k simple paths exist, returns as many as could be
    // found (possibly zero, if source cannot reach destination at all).
    //
    // Complexity: O(k * V * (E log V)) — each of the k iterations may
    // run a Dijkstra search (O(E log V)) once per node on the previous
    // path (O(V) such nodes).
    static std::vector<PathResult> kShortestPaths(
        const Graph& graph,
        int source,
        int destination,
        int k
    );

    // Builds a layered copy of a dummy-bus-node transformed graph,
    // representing the state-space (node, buses boarded so far).
    //
    // Given the original transformed graph with V nodes, the layered
    // graph has V * (maxBoardings + 1) nodes: one full copy of the
    // original graph per "layer" 0, 1, ..., maxBoardings.
    //
    //   - An original weight-0 edge (ride the same bus to another
    //     stop) becomes a same-layer edge: it doesn't change how many
    //     buses have been boarded.
    //   - An original weight-1 edge (board a bus) becomes a
    //     cross-layer edge from layer L to layer L + 1: it advances
    //     the "buses boarded" count by one. This edge is only added
    //     if layer L + 1 exists (<= maxBoardings); once the boarding
    //     budget is exhausted, no further boarding is possible.
    //
    // Use layeredNodeId() to map an original node + layer into this
    // graph's node IDs.
    static Graph buildLayeredGraph(
        const Graph& transformedGraph,
        int maxBoardings
    );

    // Maps a node ID in the original transformed graph, at a given
    // layer, to its corresponding node ID in a graph built by
    // buildLayeredGraph() with the same originalNumNodes.
    static int layeredNodeId(
        int originalNodeId,
        int layer,
        int originalNumNodes
    );

    // Returns true if destinationStop is reachable from sourceStop
    // using at most maxTransfers bus transfers (equivalently, at most
    // maxTransfers + 1 buses boarded), using the layered state-space
    // graph described above rather than simply capping zeroOneBFS's
    // result — this generalizes to variants where different classes of
    // transfer might carry different costs or restrictions.
    static bool reachableWithinTransfers(
        const Graph& transformedGraph,
        int sourceStop,
        int destinationStop,
        int maxTransfers
    );
};