#include "Graph.hpp"
#include "BusNetwork.hpp"
#include "GraphTransformer.hpp"

#include <iostream>

int main() {
    std::cout << "===== BUS NETWORK =====\n";

    // Stops have IDs 0 to 11.
    BusNetwork network(12);

    network.addBusRoute({1, 3, 7});
    network.addBusRoute({4, 7, 9, 10});
    network.addBusRoute({2, 1, 10});
    network.addBusRoute({8, 4, 11, 10});

    std::cout << "Stops: " << network.numStops() << '\n';
    std::cout << "Buses: " << network.numBuses() << "\n\n";

    for (int bus = 0; bus < network.numBuses(); ++bus) {
        std::cout << "Bus " << bus << ": ";

        for (int stop : network.stopsForBus(bus)) {
            std::cout << stop << ' ';
        }

        std::cout << '\n';
    }

    std::cout << "\n===== TRANSFORMED GRAPH =====\n";

    Graph transformedGraph = GraphTransformer::transform(network);

    std::cout << "Total graph nodes: "
              << transformedGraph.numNodes() << "\n\n";

    for (int node = 0; node < transformedGraph.numNodes(); ++node) {

        if (node < network.numStops()) {
            std::cout << "Stop Node " << node << ":\n";
        } else {
            int busId = node - network.numStops();
            std::cout << "Dummy Bus Node "
                      << node
                      << " (Bus "
                      << busId
                      << "):\n";
        }

        const auto& edges = transformedGraph.neighbors(node);

        if (edges.empty()) {
            std::cout << "  (no outgoing edges)\n";
        }

        for (const Edge& edge : edges) {

            std::cout << "  -> ";

            if (edge.to < network.numStops()) {
                std::cout << "Stop " << edge.to;
            } else {
                int busId = edge.to - network.numStops();

                std::cout << "Dummy Bus "
                          << edge.to
                          << " (Bus "
                          << busId
                          << ")";
            }

            std::cout << " [weight "
                      << edge.weight
                      << "]\n";
        }

        std::cout << '\n';
    }

    return 0;
}