#include "Algorithms.hpp"

#include <deque>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Validates that a node ID is within range for the given graph.
void validateNode(int node, int numNodes, const std::string& paramName) {
    if (node < 0 || node >= numNodes) {
        throw std::out_of_range(
            "Algorithms::zeroOneBFS: " + paramName + " " +
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
            "Algorithms::zeroOneBFS: encountered edge weight " +
            std::to_string(weight) +
            ", but 0-1 BFS only supports edge weights of 0 or 1"
        );
    }
}

} // namespace

int Algorithms::zeroOneBFS(
    const Graph& graph,
    int source,
    int destination
) {
    const int numNodes = graph.numNodes();

    validateNode(source, numNodes, "source");
    validateNode(destination, numNodes, "destination");

    constexpr int kInfinity = std::numeric_limits<int>::max();

    std::vector<int> distance(numNodes, kInfinity);
    distance[source] = 0;

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

                if (edge.weight == 0) {
                    nodeQueue.push_front(edge.to);
                } else {
                    nodeQueue.push_back(edge.to);
                }
            }
        }
    }

    if (distance[destination] == kInfinity) {
        return -1;
    }

    return distance[destination];
}