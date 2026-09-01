#include "RouteAlgorithms.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>

namespace {

// Builds a copy of `graph` that excludes the given edges and excludes
// all edges incident to the given nodes (used by Yen's algorithm to
// prevent re-discovering previously found paths or revisiting nodes
// already used in the current root path).
Graph buildFilteredGraph(
    const Graph& graph,
    const std::set<std::pair<int, int>>& removedEdges,
    const std::set<int>& removedNodes
) {
    Graph filtered(graph.numNodes());

    for (int node = 0; node < graph.numNodes(); ++node) {
        if (removedNodes.count(node) > 0) {
            continue;
        }

        for (const Edge& edge : graph.neighbors(node)) {
            if (removedNodes.count(edge.to) > 0) {
                continue;
            }

            if (removedEdges.count({node, edge.to}) > 0) {
                continue;
            }

            filtered.addDirectedEdge(node, edge.to, edge.weight);
        }
    }

    return filtered;
}

// Sums the edge weights along a given sequence of nodes, using the
// first matching edge found between each consecutive pair.
int computePathCost(const Graph& graph, const std::vector<int>& path) {
    int cost = 0;

    for (std::size_t i = 0; i + 1 < path.size(); ++i) {
        int from = path[i];
        int to = path[i + 1];

        bool found = false;

        for (const Edge& edge : graph.neighbors(from)) {
            if (edge.to == to) {
                cost += edge.weight;
                found = true;
                break;
            }
        }

        if (!found) {
            throw std::invalid_argument(
                "RouteAlgorithms: path references an edge that does "
                "not exist in the graph"
            );
        }
    }

    return cost;
}

} // namespace

std::vector<PathResult> RouteAlgorithms::kShortestPaths(
    const Graph& graph,
    int source,
    int destination,
    int k
) {
    if (k <= 0) {
        throw std::invalid_argument(
            "RouteAlgorithms::kShortestPaths: k must be positive"
        );
    }

    std::vector<PathResult> foundPaths;

    PathResult shortest = Algorithms::dijkstra(graph, source, destination);
    if (shortest.cost == -1) {
        return foundPaths;
    }

    foundPaths.push_back(shortest);

    // Candidate next-shortest paths not yet accepted into foundPaths.
    std::vector<PathResult> candidates;

    for (int iteration = 1; iteration < k; ++iteration) {
        const std::vector<int>& previousPath = foundPaths.back().path;

        for (std::size_t i = 0; i + 1 < previousPath.size(); ++i) {
            int spurNode = previousPath[i];
            std::vector<int> rootPath(
                previousPath.begin(), previousPath.begin() + i + 1
            );

            // Remove the edge that continues any already-found path
            // sharing this same root path, so Dijkstra is forced to
            // find a genuinely different continuation.
            std::set<std::pair<int, int>> removedEdges;

            for (const PathResult& found : foundPaths) {
                if (found.path.size() > i + 1 &&
                    std::equal(
                        rootPath.begin(), rootPath.end(), found.path.begin()
                    )) {
                    removedEdges.insert({found.path[i], found.path[i + 1]});
                }
            }

            // Remove root path nodes (except the spur node itself) so
            // the spur path cannot loop back through the root.
            std::set<int> removedNodes(
                rootPath.begin(), rootPath.end() - 1
            );

            Graph spurGraph = buildFilteredGraph(
                graph, removedEdges, removedNodes
            );

            PathResult spurResult =
                Algorithms::dijkstra(spurGraph, spurNode, destination);

            if (spurResult.cost == -1) {
                continue;
            }

            // Total path = root path (minus duplicated spur node) +
            // spur path.
            std::vector<int> totalPath(rootPath.begin(), rootPath.end() - 1);
            totalPath.insert(
                totalPath.end(), spurResult.path.begin(), spurResult.path.end()
            );

            int totalCost = computePathCost(graph, rootPath) + spurResult.cost;

            bool alreadyKnown = false;
            for (const PathResult& found : foundPaths) {
                if (found.path == totalPath) {
                    alreadyKnown = true;
                    break;
                }
            }
            for (const PathResult& candidate : candidates) {
                if (candidate.path == totalPath) {
                    alreadyKnown = true;
                    break;
                }
            }

            if (!alreadyKnown) {
                candidates.push_back(PathResult{totalCost, totalPath});
            }
        }

        if (candidates.empty()) {
            // No more alternative simple paths exist.
            break;
        }

        auto bestIt = std::min_element(
            candidates.begin(),
            candidates.end(),
            [](const PathResult& a, const PathResult& b) {
                return a.cost < b.cost;
            }
        );

        foundPaths.push_back(*bestIt);
        candidates.erase(bestIt);
    }

    return foundPaths;
}

Graph RouteAlgorithms::buildLayeredGraph(
    const Graph& transformedGraph,
    int maxBoardings
) {
    if (maxBoardings < 0) {
        throw std::invalid_argument(
            "RouteAlgorithms::buildLayeredGraph: maxBoardings cannot be "
            "negative"
        );
    }

    const int originalNumNodes = transformedGraph.numNodes();
    const int numLayers = maxBoardings + 1;

    Graph layeredGraph(originalNumNodes * numLayers);

    for (int layer = 0; layer < numLayers; ++layer) {
        for (int node = 0; node < originalNumNodes; ++node) {
            int fromLayered = layeredNodeId(node, layer, originalNumNodes);

            for (const Edge& edge : transformedGraph.neighbors(node)) {
                if (edge.weight == 0) {
                    // Riding the same bus to another stop: stays
                    // within the current "buses boarded" layer.
                    int toLayered =
                        layeredNodeId(edge.to, layer, originalNumNodes);
                    layeredGraph.addDirectedEdge(fromLayered, toLayered, 0);
                } else {
                    // Boarding a bus: advances to the next layer, if
                    // the boarding budget allows it.
                    if (layer + 1 < numLayers) {
                        int toLayered = layeredNodeId(
                            edge.to, layer + 1, originalNumNodes
                        );
                        layeredGraph.addDirectedEdge(
                            fromLayered, toLayered, 1
                        );
                    }
                }
            }
        }
    }

    return layeredGraph;
}

int RouteAlgorithms::layeredNodeId(
    int originalNodeId,
    int layer,
    int originalNumNodes
) {
    return layer * originalNumNodes + originalNodeId;
}

bool RouteAlgorithms::reachableWithinTransfers(
    const Graph& transformedGraph,
    int sourceStop,
    int destinationStop,
    int maxTransfers
) {
    if (maxTransfers < 0) {
        throw std::invalid_argument(
            "RouteAlgorithms::reachableWithinTransfers: maxTransfers "
            "cannot be negative"
        );
    }

    // maxTransfers transfers means at most (maxTransfers + 1) buses
    // boarded in total (the first boarding is not itself a transfer).
    const int maxBoardings = maxTransfers + 1;

    Graph layeredGraph = buildLayeredGraph(transformedGraph, maxBoardings);
    const int originalNumNodes = transformedGraph.numNodes();

    int sourceLayered = layeredNodeId(sourceStop, 0, originalNumNodes);

    for (int layer = 0; layer <= maxBoardings; ++layer) {
        int destinationLayered =
            layeredNodeId(destinationStop, layer, originalNumNodes);

        int cost = Algorithms::zeroOneBFS(
            layeredGraph, sourceLayered, destinationLayered
        );

        if (cost != -1) {
            return true;
        }
    }

    return false;
}