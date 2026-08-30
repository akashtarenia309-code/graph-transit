#include "Algorithms.hpp"
#include "BusNetwork.hpp"
#include "GraphTransformer.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

// Prints the minimum number of buses required to travel from source to
// destination, using 0-1 BFS on the transformed graph.
void printMinimumBuses(
    const Graph& graph,
    int source,
    int destination
) {
    int result = Algorithms::zeroOneBFS(graph, source, destination);

    std::cout << "Minimum buses from Stop " << source
              << " to Stop " << destination << ": ";

    if (result == -1) {
        std::cout << "Unreachable\n";
    } else {
        std::cout << result << "\n";
    }
}

int main() {
    // ----------------------------------------------------------------
    // Build the raw bus network.
    // ----------------------------------------------------------------
    BusNetwork network(12);

    network.addBusRoute({1, 3, 7});
    network.addBusRoute({4, 7, 9, 10});
    network.addBusRoute({2, 1, 10});
    network.addBusRoute({8, 4, 11, 10});

    std::cout << "===== BUS NETWORK =====\n\n";
    std::cout << "Stops: " << network.numStops() << "\n";
    std::cout << "Buses: " << network.numBuses() << "\n\n";

    for (int bus = 0; bus < network.numBuses(); ++bus) {
        std::cout << "Bus " << bus << ": ";

        const std::vector<int>& stops = network.stopsForBus(bus);
        for (std::size_t i = 0; i < stops.size(); ++i) {
            std::cout << stops[i];
            if (i + 1 < stops.size()) {
                std::cout << " -> ";
            }
        }

        std::cout << "\n";
    }

    // ----------------------------------------------------------------
    // Transform the network into a graph using dummy bus nodes.
    // ----------------------------------------------------------------
    Graph transformedGraph = GraphTransformer::transform(network);

    std::cout << "\n===== TRANSFORMED GRAPH =====\n\n";
    std::cout << "Total nodes: " << transformedGraph.numNodes() << "\n\n";

    const int numStops = network.numStops();

    for (int node = 0; node < transformedGraph.numNodes(); ++node) {
        if (node < numStops) {
            std::cout << "Stop Node " << node << ":\n";
        } else {
            int busId = node - numStops;
            std::cout << "Dummy Bus Node " << node
                      << " (Bus " << busId << "):\n";
        }

        for (const Edge& edge : transformedGraph.neighbors(node)) {
            if (edge.to < numStops) {
                std::cout << "  -> Stop " << edge.to;
            } else {
                int busId = edge.to - numStops;
                std::cout << "  -> Dummy Bus " << edge.to
                           << " (Bus " << busId << ")";
            }

            std::cout << " (weight " << edge.weight << ")\n";
        }
    }

    // ----------------------------------------------------------------
    // Query the minimum number of buses required for several routes.
    // ----------------------------------------------------------------
    std::cout << "\n===== MINIMUM BUS QUERIES =====\n\n";

    printMinimumBuses(transformedGraph, 1, 9); // Expected: 2
    printMinimumBuses(transformedGraph, 2, 7); // Expected: 2
    printMinimumBuses(transformedGraph, 8, 3); // Expected: 3
    printMinimumBuses(transformedGraph, 1, 1); // Expected: 0
    printMinimumBuses(transformedGraph, 0, 7); // Expected: Unreachable

    return 0;
}