#pragma once

#include <vector>

// Represents a single directed edge in the adjacency list.
struct Edge {
    int to;
    int weight;
};

// General-purpose weighted graph using an adjacency list representation.
class Graph {
public:
    // Constructs a graph with node IDs from 0 to numNodes - 1.
    explicit Graph(int numNodes);

    // Adds a directed weighted edge: from -> to.
    void addDirectedEdge(int from, int to, int weight);

    // Adds an undirected weighted edge between u and v.
    void addUndirectedEdge(int u, int v, int weight);

    // Returns the outgoing edges of a node.
    const std::vector<Edge>& neighbors(int node) const;

    // Returns the total number of nodes in the graph.
    int numNodes() const;

private:
    std::vector<std::vector<Edge>> adjacencyList_;
};