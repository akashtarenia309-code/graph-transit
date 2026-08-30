#include "Graph.hpp"

#include <iostream>

int main() {
    // Create a graph with 5 nodes: IDs 0, 1, 2, 3, 4
    Graph graph(5);

    // Directed weighted edges
    graph.addDirectedEdge(0, 1, 4);
    graph.addDirectedEdge(0, 2, 1);
    graph.addDirectedEdge(2, 3, 7);
    graph.addDirectedEdge(3, 4, 2);

    // Undirected weighted edge
    graph.addUndirectedEdge(1, 4, 5);

    std::cout << "Total number of nodes: "
              << graph.numNodes() << "\n\n";

    for (int node = 0; node < graph.numNodes(); ++node) {
        std::cout << "Node " << node << ":\n";

        const std::vector<Edge>& edges = graph.neighbors(node);

        if (edges.empty()) {
            std::cout << "  (no outgoing edges)\n";
        }

        for (const Edge& edge : edges) {
            std::cout << "  -> " << edge.to
                      << " (weight " << edge.weight << ")\n";
        }
    }

    return 0;
}