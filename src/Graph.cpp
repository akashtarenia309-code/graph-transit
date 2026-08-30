#include "Graph.hpp"

#include <stdexcept>
#include <string>

namespace {

// Validates that a node ID exists in the graph.
void validateNode(int node, int numNodes) {
    if (node < 0 || node >= numNodes) {
        throw std::out_of_range(
            "Graph: node id " + std::to_string(node) +
            " is out of range for a graph with " +
            std::to_string(numNodes) + " nodes"
        );
    }
}

} // namespace

Graph::Graph(int numNodes) {
    if (numNodes < 0) {
        throw std::invalid_argument(
            "Graph: number of nodes cannot be negative"
        );
    }

    adjacencyList_.resize(numNodes);
}

void Graph::addDirectedEdge(int from, int to, int weight) {
    validateNode(from, numNodes());
    validateNode(to, numNodes());

    adjacencyList_[from].push_back({to, weight});
}

void Graph::addUndirectedEdge(int u, int v, int weight) {
    addDirectedEdge(u, v, weight);
    addDirectedEdge(v, u, weight);
}

const std::vector<Edge>& Graph::neighbors(int node) const {
    validateNode(node, numNodes());

    return adjacencyList_[node];
}

int Graph::numNodes() const {
    return static_cast<int>(adjacencyList_.size());
}