#include "FlowNetwork.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>

FlowNetwork::FlowNetwork(int numNodes) : numNodes_(numNodes) {
    if (numNodes < 0) {
        throw std::invalid_argument(
            "FlowNetwork: number of nodes cannot be negative"
        );
    }

    adjacency_.resize(numNodes);
}

void FlowNetwork::addEdge(int from, int to, long long capacity) {
    if (from < 0 || from >= numNodes_ || to < 0 || to >= numNodes_) {
        throw std::out_of_range(
            "FlowNetwork: edge endpoint out of range for a network with " +
            std::to_string(numNodes_) + " nodes"
        );
    }

    if (capacity < 0) {
        throw std::invalid_argument(
            "FlowNetwork: edge capacity cannot be negative"
        );
    }

    // Forward edge and its paired reverse residual edge (capacity 0
    // initially — it gains capacity as flow is pushed forward).
    int forwardIndex = static_cast<int>(edges_.size());
    edges_.push_back(FlowEdge{to, capacity, 0});
    adjacency_[from].push_back(forwardIndex);

    int reverseIndex = static_cast<int>(edges_.size());
    edges_.push_back(FlowEdge{from, 0, 0});
    adjacency_[to].push_back(reverseIndex);
}

bool FlowNetwork::buildLevelGraph(
    int source,
    int sink,
    std::vector<int>& level
) const {
    level.assign(numNodes_, -1);
    level[source] = 0;

    std::queue<int> nodeQueue;
    nodeQueue.push(source);

    while (!nodeQueue.empty()) {
        int node = nodeQueue.front();
        nodeQueue.pop();

        for (int edgeIndex : adjacency_[node]) {
            const FlowEdge& edge = edges_[edgeIndex];

            bool hasResidualCapacity = (edge.capacity - edge.flow) > 0;

            if (hasResidualCapacity && level[edge.to] == -1) {
                level[edge.to] = level[node] + 1;
                nodeQueue.push(edge.to);
            }
        }
    }

    return level[sink] != -1;
}

long long FlowNetwork::sendBlockingFlow(
    int node,
    int sink,
    long long pushed,
    std::vector<int>& level,
    std::vector<int>& iter
) {
    if (node == sink) {
        return pushed;
    }

    // "Current arc" optimization: resume from where we left off for
    // this node instead of rescanning already-exhausted edges.
    for (int& i = iter[node]; i < static_cast<int>(adjacency_[node].size()); ++i) {
        int edgeIndex = adjacency_[node][i];
        FlowEdge& edge = edges_[edgeIndex];

        long long residual = edge.capacity - edge.flow;

        if (residual > 0 && level[edge.to] == level[node] + 1) {
            long long amount = std::min(pushed, residual);
            long long sent = sendBlockingFlow(
                edge.to, sink, amount, level, iter
            );

            if (sent > 0) {
                edge.flow += sent;
                // The paired reverse edge lives at the adjacent index
                // (forward/reverse pairs are always pushed together).
                edges_[edgeIndex ^ 1].flow -= sent;
                return sent;
            }
        }
    }

    // No augmenting path found from this node in the current level
    // graph; mark it as exhausted for this phase.
    level[node] = -1;
    return 0;
}

long long FlowNetwork::maxFlow(int source, int sink) {
    if (source < 0 || source >= numNodes_ || sink < 0 || sink >= numNodes_) {
        throw std::out_of_range(
            "FlowNetwork: source/sink out of range for a network with " +
            std::to_string(numNodes_) + " nodes"
        );
    }

    if (source == sink) {
        return 0;
    }

    long long totalFlow = 0;
    std::vector<int> level;

    while (buildLevelGraph(source, sink, level)) {
        std::vector<int> iter(numNodes_, 0);

        constexpr long long kUnbounded = std::numeric_limits<long long>::max();

        long long pushed;
        while ((pushed = sendBlockingFlow(source, sink, kUnbounded, level, iter)) > 0) {
            totalFlow += pushed;
        }
    }

    return totalFlow;
}