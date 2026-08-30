#include "Graph.hpp"
#include "BusNetwork.hpp"

#include <iostream>
#include <vector>

int main() {
    std::cout << "===== GRAPH TEST =====\n";

    Graph graph(5);

    graph.addDirectedEdge(0, 1, 4);
    graph.addDirectedEdge(0, 2, 1);
    graph.addDirectedEdge(2, 3, 7);
    graph.addDirectedEdge(3, 4, 2);
    graph.addUndirectedEdge(1, 4, 5);

    std::cout << "Total number of nodes: "
              << graph.numNodes() << "\n\n";

    for (int node = 0; node < graph.numNodes(); ++node) {
        std::cout << "Node " << node << ":\n";

        for (const Edge& edge : graph.neighbors(node)) {
            std::cout << "  -> " << edge.to
                      << " (weight " << edge.weight << ")\n";
        }
    }

    std::cout << "\n===== BUS NETWORK TEST =====\n";

    // Create a network with stops 0 to 11.
    BusNetwork network(12);

    network.addBusRoute({1, 3, 7});
    network.addBusRoute({4, 7, 9, 10});
    network.addBusRoute({2, 1, 10});
    network.addBusRoute({8, 4, 11, 10});

    std::cout << "Total stops: "
              << network.numStops() << "\n";

    std::cout << "Total buses: "
              << network.numBuses() << "\n\n";

    // Print Bus -> Stops mapping.
    std::cout << "Bus -> Stops:\n";

    for (int bus = 0; bus < network.numBuses(); ++bus) {
        std::cout << "Bus " << bus << ": ";

        for (int stop : network.stopsForBus(bus)) {
            std::cout << stop << " ";
        }

        std::cout << "\n";
    }

    // Print Stop -> Buses mapping.
    std::cout << "\nStop -> Buses:\n";

    for (int stop = 0; stop < network.numStops(); ++stop) {
        const auto& buses = network.busesForStop(stop);

        if (!buses.empty()) {
            std::cout << "Stop " << stop << ": ";

            for (int bus : buses) {
                std::cout << "Bus " << bus << " ";
            }

            std::cout << "\n";
        }
    }

    return 0;
}