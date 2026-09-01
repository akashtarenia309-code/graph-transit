#include "Algorithms.hpp"

#include <algorithm>
#include <deque>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

// Validates that a node ID is within range for the given graph.
void validateNode(int node, int numNodes, const std::string& paramName) {
    if (node < 0 || node >= numNodes) {
        throw std::out_of_range(
            "Algorithms: " + paramName + " " +
            std::to_string(node) + " is out of range for a graph with " +
            std::to_string(numNodes) + " nodes"
        );
    }
}

// Validates that an edge weight is either 0 or 1, as required for
// 0-1 BFS to produce correct results.
void validateEdgeWeight(int weight) {
    if (weight != 0 && weight != 1) {
        throw std::invalid_argument(
            "Algorithms: encountered edge weight " +
            std::to_string(weight) +
            ", but 0-1 BFS only supports edge weights of 0 or 1"
        );
    }
}

// Validates that an edge weight is non-negative, as required for
// Dijkstra's algorithm to produce correct results.
void validateNonNegativeWeight(int weight) {
    if (weight < 0) {
        throw std::invalid_argument(
            "Algorithms: encountered negative edge weight " +
            std::to_string(weight) +
            ", but Dijkstra's algorithm requires non-negative weights"
        );
    }
}

} // namespace

void Algorithms::runZeroOneBFS(
    const Graph& graph,
    int source,
    int destination,
    std::vector<int>& distance,
    std::vector<int>& parent
) {
    const int numNodes = graph.numNodes();

    validateNode(source, numNodes, "source");
    validateNode(destination, numNodes, "destination");

    constexpr int kInfinity = std::numeric_limits<int>::max();

    distance.assign(numNodes, kInfinity);
    parent.assign(numNodes, -1);

    distance[source] = 0;
    parent[source] = -1;

    std::deque<int> nodeQueue;
    nodeQueue.push_back(source);

    while (!nodeQueue.empty()) {
        int current = nodeQueue.front();
        nodeQueue.pop_front();

        for (const Edge& edge : graph.neighbors(current)) {
            validateEdgeWeight(edge.weight);

            int newDistance = distance[current] + edge.weight;

            if (newDistance < distance[edge.to]) {
                distance[edge.to] = newDistance;
                parent[edge.to] = current;

                if (edge.weight == 0) {
                    // Free move: process it before anything already
                    // queued, since it can't be beaten by a longer path.
                    nodeQueue.push_front(edge.to);
                } else {
                    // Costs one more bus: process it after everything
                    // currently known to be no more expensive.
                    nodeQueue.push_back(edge.to);
                }
            }
        }
    }
}

int Algorithms::zeroOneBFS(
    const Graph& graph,
    int source,
    int destination
) {
    std::vector<int> distance;
    std::vector<int> parent;

    runZeroOneBFS(graph, source, destination, distance, parent);

    constexpr int kInfinity = std::numeric_limits<int>::max();

    if (distance[destination] == kInfinity) {
        return -1;
    }

    return distance[destination];
}

PathResult Algorithms::zeroOneBFSWithPath(
    const Graph& graph,
    int source,
    int destination
) {
    std::vector<int> distance;
    std::vector<int> parent;

    runZeroOneBFS(graph, source, destination, distance, parent);

    constexpr int kInfinity = std::numeric_limits<int>::max();

    if (distance[destination] == kInfinity) {
        return PathResult{-1, {}};
    }

    // Reconstruct the path by walking backwards from destination to
    // source via the parent pointers, then reversing the result.
    std::vector<int> path;

    for (int current = destination; current != -1; current = parent[current]) {
        path.push_back(current);

        if (current == source) {
            break;
        }
    }

    std::reverse(path.begin(), path.end());

    return PathResult{distance[destination], path};
}

PathResult Algorithms::dijkstra(
    const Graph& graph,
    int source,
    int destination
) {
    const int numNodes = graph.numNodes();

    validateNode(source, numNodes, "source");
    validateNode(destination, numNodes, "destination");

    constexpr long long kInfinity = std::numeric_limits<long long>::max();

    std::vector<long long> distance(numNodes, kInfinity);
    std::vector<int> parent(numNodes, -1);

    distance[source] = 0;

    // Min-heap of (distance, node), smallest distance on top.
    using DistNodePair = std::pair<long long, int>;
    std::priority_queue<
        DistNodePair,
        std::vector<DistNodePair>,
        std::greater<DistNodePair>
    > pq;

    pq.push({0, source});

    while (!pq.empty()) {
        DistNodePair top = pq.top();
        long long currentDistance = top.first;
        int current = top.second;
        pq.pop();

        // Skip stale entries: a shorter distance to this node was
        // already found and processed since this entry was pushed.
        if (currentDistance > distance[current]) {
            continue;
        }

        for (const Edge& edge : graph.neighbors(current)) {
            validateNonNegativeWeight(edge.weight);

            long long newDistance = distance[current] + edge.weight;

            if (newDistance < distance[edge.to]) {
                distance[edge.to] = newDistance;
                parent[edge.to] = current;
                pq.push({newDistance, edge.to});
            }
        }
    }

    if (distance[destination] == kInfinity) {
        return PathResult{-1, {}};
    }

    // Reconstruct the path by walking backwards via parent pointers.
    std::vector<int> path;

    for (int current = destination; current != -1; current = parent[current]) {
        path.push_back(current);

        if (current == source) {
            break;
        }
    }

    std::reverse(path.begin(), path.end());

    return PathResult{static_cast<int>(distance[destination]), path};
}