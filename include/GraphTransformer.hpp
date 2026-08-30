#pragma once

#include "BusNetwork.hpp"
#include "Graph.hpp"

// GraphTransformer converts a raw BusNetwork into a Graph suitable for
// shortest-path algorithms using the dummy bus node transformation.
//
// For every bus, one dummy node represents being on that bus.
//
// For every stop visited by a bus:
//
// Stop -> Dummy Bus Node : weight 1
// Dummy Bus Node -> Stop : weight 0
//
// This allows 0-1 BFS to minimize the number of buses boarded.
//
// Node layout:
//
// Real stop nodes:
// 0 to N - 1
//
// Dummy bus nodes:
// N to N + M - 1
//
// Mapping:
// dummyBusNodeId(busId) = N + busId
class GraphTransformer {
public:
    // Converts a BusNetwork into a graph containing real stop nodes
    // and dummy bus nodes.
    static Graph transform(const BusNetwork& network);

    // Returns the graph node ID representing a bus.
    static int dummyBusNodeId(int busId, int numStops);
};