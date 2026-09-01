#include "Algorithms.hpp"
#include "BusNetwork.hpp"
#include "FlowNetwork.hpp"
#include "Graph.hpp"
#include "GraphTransformer.hpp"
#include "RouteAlgorithms.hpp"
#include "Tarjan.hpp"

#include <iostream>
#include <vector>

// ---------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------

// Prints the minimum number of buses required to travel from source to
// destination, using 0-1 BFS on the dummy-bus-node transformed graph.
void printMinimumBuses(const Graph& graph, int source, int destination) {
    int result = Algorithms::zeroOneBFS(graph, source, destination);

    std::cout << "Minimum buses from Stop " << source
              << " to Stop " << destination << ": ";

    if (result == -1) {
        std::cout << "Unreachable\n";
    } else {
        std::cout << result << "\n";
    }
}

// Prints a node label for the transformed (dummy-bus-node) graph: a
// plain stop number, or "Dummy Bus X (Bus Y)" for a dummy node.
void printTransformedNodeLabel(int node, int numStops) {
    if (node < numStops) {
        std::cout << "Stop " << node;
    } else {
        std::cout << "Dummy Bus " << node << " (Bus " << (node - numStops) << ")";
    }
}

// Prints a PathResult over the transformed (dummy-bus-node) graph in a
// human-readable form.
void printTransformedPath(const PathResult& result, int numStops) {
    if (result.cost == -1) {
        std::cout << "  Unreachable\n";
        return;
    }

    std::cout << "  Cost: " << result.cost << " | Path: ";
    for (std::size_t i = 0; i < result.path.size(); ++i) {
        printTransformedNodeLabel(result.path[i], numStops);
        if (i + 1 < result.path.size()) {
            std::cout << " -> ";
        }
    }
    std::cout << "\n";
}

int main() {
    // ===================================================================
    // ===== BUS NETWORK =====
    // ===================================================================
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

    // ===================================================================
    // ===== TRANSFORMED GRAPH =====
    // ===================================================================
    Graph transformedGraph = GraphTransformer::transform(network);
    const int numStops = network.numStops();

    std::cout << "\n===== TRANSFORMED GRAPH =====\n\n";
    std::cout << "Total nodes: " << transformedGraph.numNodes() << "\n\n";

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

    // ===================================================================
    // ===== MINIMUM BUS ROUTING =====
    // ===================================================================
    std::cout << "\n===== MINIMUM BUS ROUTING =====\n\n";

    printMinimumBuses(transformedGraph, 1, 9); // Expected: 2
    printMinimumBuses(transformedGraph, 2, 7); // Expected: 2
    printMinimumBuses(transformedGraph, 8, 3); // Expected: 3
    printMinimumBuses(transformedGraph, 1, 1); // Expected: 0
    printMinimumBuses(transformedGraph, 0, 7); // Expected: Unreachable

    std::cout << "\nReconstructed path, Stop 8 -> Stop 3:\n";
    PathResult minBusPath = Algorithms::zeroOneBFSWithPath(transformedGraph, 8, 3);
    printTransformedPath(minBusPath, numStops);

    // ===================================================================
    // ===== DIJKSTRA FASTEST ROUTE =====
    // ===================================================================
    // A separate, independent graph modeling direct stop-to-stop travel
    // times (in minutes) for stops 0..11 -- distinct from the dummy-bus
    // graph above, since here weights represent real travel time rather
    // than "number of buses boarded".
    std::cout << "\n===== DIJKSTRA FASTEST ROUTE =====\n\n";

    Graph travelTimeGraph(numStops);
    travelTimeGraph.addUndirectedEdge(1, 3, 6);
    travelTimeGraph.addUndirectedEdge(3, 7, 4);
    travelTimeGraph.addUndirectedEdge(4, 7, 5);
    travelTimeGraph.addUndirectedEdge(7, 9, 3);
    travelTimeGraph.addUndirectedEdge(9, 10, 2);
    travelTimeGraph.addUndirectedEdge(2, 1, 7);
    travelTimeGraph.addUndirectedEdge(1, 10, 15);
    travelTimeGraph.addUndirectedEdge(8, 4, 6);
    travelTimeGraph.addUndirectedEdge(4, 11, 4);
    travelTimeGraph.addUndirectedEdge(11, 10, 3);

    PathResult fastestRoute = Algorithms::dijkstra(travelTimeGraph, 8, 9);

    std::cout << "Fastest route, Stop 8 -> Stop 9:\n";
    if (fastestRoute.cost == -1) {
        std::cout << "  Unreachable\n";
    } else {
        std::cout << "  Total time: " << fastestRoute.cost << " minutes | Path: ";
        for (std::size_t i = 0; i < fastestRoute.path.size(); ++i) {
            std::cout << "Stop " << fastestRoute.path[i];
            if (i + 1 < fastestRoute.path.size()) {
                std::cout << " -> ";
            }
        }
        std::cout << "\n";
    }

    // ===================================================================
    // ===== CRITICAL STOPS =====
    // ===================================================================
    // Undirected connectivity graph: an edge between two stops that are
    // directly connected by a bus route (any weight, since Tarjan only
    // cares about connectivity, not cost).
    std::cout << "\n===== CRITICAL STOPS =====\n\n";

    Graph connectivityGraph(numStops);
    connectivityGraph.addUndirectedEdge(1, 3, 1);
    connectivityGraph.addUndirectedEdge(3, 7, 1);
    connectivityGraph.addUndirectedEdge(4, 7, 1);
    connectivityGraph.addUndirectedEdge(7, 9, 1);
    connectivityGraph.addUndirectedEdge(9, 10, 1);
    connectivityGraph.addUndirectedEdge(2, 1, 1);
    connectivityGraph.addUndirectedEdge(1, 10, 1);
    connectivityGraph.addUndirectedEdge(8, 4, 1);
    connectivityGraph.addUndirectedEdge(4, 11, 1);
    connectivityGraph.addUndirectedEdge(11, 10, 1);

    TarjanResult criticalInfrastructure = Tarjan::analyze(connectivityGraph);

    std::cout << "Connected components: "
              << criticalInfrastructure.connectedComponents.size() << "\n\n";

    std::cout << "Articulation points (critical stops):\n";
    if (criticalInfrastructure.articulationPoints.empty()) {
        std::cout << "  None\n";
    } else {
        for (int stop : criticalInfrastructure.articulationPoints) {
            std::cout << "  Stop " << stop << "\n";
        }
    }

    std::cout << "\nBridges (critical connections):\n";
    if (criticalInfrastructure.bridges.empty()) {
        std::cout << "  None\n";
    } else {
        for (const auto& bridge : criticalInfrastructure.bridges) {
            std::cout << "  Stop " << bridge.first
                      << " <-> Stop " << bridge.second << "\n";
        }
    }

    // ===================================================================
    // ===== MAXIMUM PASSENGER FLOW =====
    // ===================================================================
    // Same stop layout, but now edges carry passenger-capacity-per-unit-
    // time instead of a travel time or a pure connectivity flag.
    std::cout << "\n===== MAXIMUM PASSENGER FLOW =====\n\n";

    FlowNetwork flowNetwork(numStops);
    flowNetwork.addEdge(8, 4, 50);
    flowNetwork.addEdge(4, 7, 30);
    flowNetwork.addEdge(4, 11, 25);
    flowNetwork.addEdge(11, 10, 40);
    flowNetwork.addEdge(7, 9, 20);
    flowNetwork.addEdge(7, 3, 15);
    flowNetwork.addEdge(9, 10, 20);
    flowNetwork.addEdge(3, 1, 15);
    flowNetwork.addEdge(1, 10, 10);

    long long maxPassengerFlow = flowNetwork.maxFlow(8, 10);

    std::cout << "Maximum passenger flow, Stop 8 -> Stop 10: "
              << maxPassengerFlow << " passengers/unit time\n";

    // ===================================================================
    // ===== K SHORTEST ROUTES =====
    // ===================================================================
    std::cout << "\n===== K SHORTEST ROUTES =====\n\n";

    const int k = 3;
    std::vector<PathResult> kRoutes =
        RouteAlgorithms::kShortestPaths(transformedGraph, 8, 10, k);

    std::cout << "Top " << kRoutes.size()
              << " routes (by buses boarded), Stop 8 -> Stop 10:\n";

    for (std::size_t i = 0; i < kRoutes.size(); ++i) {
        std::cout << i + 1 << ") ";
        printTransformedPath(kRoutes[i], numStops);
    }

    // ===================================================================
    // ===== LIMITED TRANSFER ROUTING =====
    // ===================================================================
    std::cout << "\n===== LIMITED TRANSFER ROUTING =====\n\n";

    for (int maxTransfers = 0; maxTransfers <= 2; ++maxTransfers) {
        bool reachable = RouteAlgorithms::reachableWithinTransfers(
            transformedGraph, 8, 3, maxTransfers
        );

        std::cout << "Stop 8 -> Stop 3 within " << maxTransfers
                  << " transfer(s): "
                  << (reachable ? "Reachable" : "Not reachable") << "\n";
    }

    return 0;
}