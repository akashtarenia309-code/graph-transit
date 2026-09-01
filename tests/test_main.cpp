// Lightweight assertion-based test suite for GraphTransit.
//
// No external test framework is used, to keep the project dependency-
// free — each test is a small function that exercises one module and
// reports pass/fail. main() runs them all and returns non-zero if any
// test failed, so this integrates cleanly with `ctest`.

#include "Algorithms.hpp"
#include "BusNetwork.hpp"
#include "FlowNetwork.hpp"
#include "Graph.hpp"
#include "GraphTransformer.hpp"
#include "RouteAlgorithms.hpp"
#include "Tarjan.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int g_failures = 0;

void check(bool condition, const std::string& description) {
    if (condition) {
        std::cout << "[PASS] " << description << "\n";
    } else {
        std::cout << "[FAIL] " << description << "\n";
        ++g_failures;
    }
}

BusNetwork buildSampleNetwork() {
    BusNetwork network(12);
    network.addBusRoute({1, 3, 7});
    network.addBusRoute({4, 7, 9, 10});
    network.addBusRoute({2, 1, 10});
    network.addBusRoute({8, 4, 11, 10});
    return network;
}

// -----------------------------------------------------------------
// Graph
// -----------------------------------------------------------------
void testGraph() {
    Graph graph(4);
    graph.addDirectedEdge(0, 1, 5);
    graph.addUndirectedEdge(1, 2, 3);

    check(graph.numNodes() == 4, "Graph: numNodes returns constructor value");
    check(graph.neighbors(0).size() == 1, "Graph: directed edge added once");
    check(graph.neighbors(1).size() == 1, "Graph: undirected edge visible from node 1's side");
    check(graph.neighbors(2).size() == 1, "Graph: undirected edge visible from node 2's side");

    bool threw = false;
    try {
        graph.neighbors(10);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    check(threw, "Graph: out-of-range node throws std::out_of_range");
}

// -----------------------------------------------------------------
// BusNetwork
// -----------------------------------------------------------------
void testBusNetwork() {
    BusNetwork network = buildSampleNetwork();

    check(network.numStops() == 12, "BusNetwork: numStops matches constructor");
    check(network.numBuses() == 4, "BusNetwork: numBuses matches routes added");
    check(network.stopsForBus(0) == std::vector<int>({1, 3, 7}),
          "BusNetwork: stopsForBus returns correct ordered route");

    const std::vector<int>& busesAtStop1 = network.busesForStop(1);
    check(busesAtStop1.size() == 2,
          "BusNetwork: busesForStop finds all buses visiting a stop");
}

// -----------------------------------------------------------------
// GraphTransformer + zeroOneBFS (minimum bus boarding)
// -----------------------------------------------------------------
void testMinimumBusBoarding() {
    BusNetwork network = buildSampleNetwork();
    Graph transformed = GraphTransformer::transform(network);

    check(transformed.numNodes() == network.numStops() + network.numBuses(),
          "GraphTransformer: node count is stops + buses");

    check(Algorithms::zeroOneBFS(transformed, 1, 9) == 2,
          "zeroOneBFS: Stop 1 -> Stop 9 costs 2 buses");
    check(Algorithms::zeroOneBFS(transformed, 8, 3) == 3,
          "zeroOneBFS: Stop 8 -> Stop 3 costs 3 buses");
    check(Algorithms::zeroOneBFS(transformed, 1, 1) == 0,
          "zeroOneBFS: source == destination costs 0");
    check(Algorithms::zeroOneBFS(transformed, 0, 7) == -1,
          "zeroOneBFS: isolated stop is unreachable");

    PathResult pathResult = Algorithms::zeroOneBFSWithPath(transformed, 8, 3);
    check(pathResult.cost == 3, "zeroOneBFSWithPath: cost matches zeroOneBFS");
    check(pathResult.path.front() == 8 && pathResult.path.back() == 3,
          "zeroOneBFSWithPath: path starts at source and ends at destination");
}

// -----------------------------------------------------------------
// Dijkstra
// -----------------------------------------------------------------
void testDijkstra() {
    Graph graph(4);
    graph.addDirectedEdge(0, 1, 1);
    graph.addDirectedEdge(1, 2, 2);
    graph.addDirectedEdge(0, 2, 10);
    graph.addDirectedEdge(2, 3, 1);

    PathResult result = Algorithms::dijkstra(graph, 0, 3);
    check(result.cost == 4, "dijkstra: finds shorter multi-hop path over direct edge");
    check(result.path == std::vector<int>({0, 1, 2, 3}),
          "dijkstra: reconstructs correct path");

    PathResult unreachable = Algorithms::dijkstra(graph, 3, 0);
    check(unreachable.cost == -1, "dijkstra: reports unreachable destination");
}

// -----------------------------------------------------------------
// Tarjan (articulation points / bridges)
// -----------------------------------------------------------------
void testTarjan() {
    // A simple "bowtie" graph: 0-1-2 form a triangle, 2-3 is a bridge,
    // 3-4-5 form another triangle. Node 2 and 3 are articulation
    // points, and (2,3) is the only bridge.
    Graph graph(6);
    graph.addUndirectedEdge(0, 1, 1);
    graph.addUndirectedEdge(1, 2, 1);
    graph.addUndirectedEdge(2, 0, 1);
    graph.addUndirectedEdge(2, 3, 1);
    graph.addUndirectedEdge(3, 4, 1);
    graph.addUndirectedEdge(4, 5, 1);
    graph.addUndirectedEdge(5, 3, 1);

    TarjanResult result = Tarjan::analyze(graph);

    bool has2 = false;
    bool has3 = false;
    for (int node : result.articulationPoints) {
        if (node == 2) has2 = true;
        if (node == 3) has3 = true;
    }
    check(has2 && has3, "Tarjan: identifies both articulation points in a bowtie graph");

    bool hasBridge = false;
    for (const auto& bridge : result.bridges) {
        if ((bridge.first == 2 && bridge.second == 3) ||
            (bridge.first == 3 && bridge.second == 2)) {
            hasBridge = true;
        }
    }
    check(hasBridge, "Tarjan: identifies the connecting edge as a bridge");
    check(result.connectedComponents.size() == 1,
          "Tarjan: bowtie graph is a single connected component");
}

// -----------------------------------------------------------------
// FlowNetwork (Dinic's algorithm)
// -----------------------------------------------------------------
void testFlowNetwork() {
    // Classic small max-flow example: source 0, sink 3.
    FlowNetwork flow(4);
    flow.addEdge(0, 1, 3);
    flow.addEdge(0, 2, 2);
    flow.addEdge(1, 2, 1);
    flow.addEdge(1, 3, 2);
    flow.addEdge(2, 3, 3);

    long long maxFlowValue = flow.maxFlow(0, 3);
    check(maxFlowValue == 5, "FlowNetwork: computes correct max flow value");

    FlowNetwork disconnected(3);
    disconnected.addEdge(0, 1, 5);
    check(disconnected.maxFlow(0, 2) == 0,
          "FlowNetwork: disconnected sink yields zero flow");
}

// -----------------------------------------------------------------
// RouteAlgorithms: Yen's K shortest paths
// -----------------------------------------------------------------
void testKShortestPaths() {
    BusNetwork network = buildSampleNetwork();
    Graph transformed = GraphTransformer::transform(network);

    std::vector<PathResult> routes =
        RouteAlgorithms::kShortestPaths(transformed, 8, 10, 3);

    check(!routes.empty(), "kShortestPaths: finds at least one route");
    check(routes.front().cost == 1,
          "kShortestPaths: first route is the true shortest (direct bus)");

    for (std::size_t i = 1; i < routes.size(); ++i) {
        check(routes[i].cost >= routes[i - 1].cost,
              "kShortestPaths: routes are returned in non-decreasing cost order");
    }
}

// -----------------------------------------------------------------
// RouteAlgorithms: layered graph limited-transfer routing
// -----------------------------------------------------------------
void testLimitedTransferRouting() {
    BusNetwork network = buildSampleNetwork();
    Graph transformed = GraphTransformer::transform(network);

    // Stop 8 -> Stop 3 requires 3 buses (2 transfers).
    check(!RouteAlgorithms::reachableWithinTransfers(transformed, 8, 3, 0),
          "reachableWithinTransfers: not reachable with 0 transfers");
    check(!RouteAlgorithms::reachableWithinTransfers(transformed, 8, 3, 1),
          "reachableWithinTransfers: not reachable with 1 transfer");
    check(RouteAlgorithms::reachableWithinTransfers(transformed, 8, 3, 2),
          "reachableWithinTransfers: reachable with 2 transfers");
}

} // namespace

int main() {
    testGraph();
    testBusNetwork();
    testMinimumBusBoarding();
    testDijkstra();
    testTarjan();
    testFlowNetwork();
    testKShortestPaths();
    testLimitedTransferRouting();

    std::cout << "\n";
    if (g_failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cout << g_failures << " test(s) failed.\n";
    return 1;
}