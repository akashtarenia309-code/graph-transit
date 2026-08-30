#include "GraphTransformer.hpp"

#include <stdexcept>

int GraphTransformer::dummyBusNodeId(int busId, int numStops) {
    if (busId < 0) {
        throw std::invalid_argument(
            "GraphTransformer: busId cannot be negative"
        );
    }

    if (numStops < 0) {
        throw std::invalid_argument(
            "GraphTransformer: numStops cannot be negative"
        );
    }

    return numStops + busId;
}

Graph GraphTransformer::transform(const BusNetwork& network) {
    const int numStops = network.numStops();
    const int numBuses = network.numBuses();

    // One node per real stop, plus one dummy node per bus.
    Graph graph(numStops + numBuses);

    for (int busId = 0; busId < numBuses; ++busId) {
        const int dummyNode = dummyBusNodeId(busId, numStops);

        for (int stop : network.stopsForBus(busId)) {
            // Boarding this bus from a stop costs 1.
            graph.addDirectedEdge(stop, dummyNode, 1);

            // Once on the bus, reaching any stop on its route costs 0.
            graph.addDirectedEdge(dummyNode, stop, 0);
        }
    }

    return graph;
}