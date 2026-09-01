#include "Tarjan.hpp"

#include <algorithm>

void Tarjan::dfs(
    const Graph& graph,
    int node,
    int parent,
    int& timer,
    std::vector<int>& discoveryTime,
    std::vector<int>& lowLink,
    std::vector<bool>& visited,
    std::vector<bool>& isArticulationPoint,
    std::vector<std::pair<int, int>>& bridges,
    std::vector<int>& currentComponent
) {
    visited[node] = true;
    currentComponent.push_back(node);

    discoveryTime[node] = timer;
    lowLink[node] = timer;
    ++timer;

    int childCount = 0;

    // Whether we've already skipped one edge back to `parent`. Only
    // skip the parent edge once, in case there are legitimately two
    // separate direct connections between the same pair of stops
    // (which would not be simple graphs, but this guards against it).
    bool skippedParentEdge = false;

    for (const Edge& edge : graph.neighbors(node)) {
        int neighbor = edge.to;

        if (neighbor == parent && !skippedParentEdge) {
            skippedParentEdge = true;
            continue;
        }

        if (!visited[neighbor]) {
            ++childCount;

            dfs(
                graph,
                neighbor,
                node,
                timer,
                discoveryTime,
                lowLink,
                visited,
                isArticulationPoint,
                bridges,
                currentComponent
            );

            lowLink[node] = std::min(lowLink[node], lowLink[neighbor]);

            // Articulation point rule for a non-root node: node cuts
            // off `neighbor`'s subtree if that subtree cannot reach
            // back above `node` in the DFS tree.
            if (parent != -1 && lowLink[neighbor] >= discoveryTime[node]) {
                isArticulationPoint[node] = true;
            }

            // Bridge rule: the edge (node, neighbor) is a bridge if
            // neighbor's subtree has no back-edge reaching node or
            // higher.
            if (lowLink[neighbor] > discoveryTime[node]) {
                bridges.push_back({node, neighbor});
            }
        } else {
            // Back edge: update low-link using the neighbor's
            // discovery time (not its low-link), per standard Tarjan.
            lowLink[node] = std::min(lowLink[node], discoveryTime[neighbor]);
        }
    }

    // Articulation point rule for the root of the DFS tree: the root
    // is critical only if it has more than one child in the DFS tree.
    if (parent == -1 && childCount > 1) {
        isArticulationPoint[node] = true;
    }
}

TarjanResult Tarjan::analyze(const Graph& graph) {
    const int numNodes = graph.numNodes();

    std::vector<int> discoveryTime(numNodes, -1);
    std::vector<int> lowLink(numNodes, -1);
    std::vector<bool> visited(numNodes, false);
    std::vector<bool> isArticulationPoint(numNodes, false);

    TarjanResult result;

    int timer = 0;

    for (int node = 0; node < numNodes; ++node) {
        if (visited[node]) {
            continue;
        }

        std::vector<int> currentComponent;

        dfs(
            graph,
            node,
            -1,
            timer,
            discoveryTime,
            lowLink,
            visited,
            isArticulationPoint,
            result.bridges,
            currentComponent
        );

        result.connectedComponents.push_back(currentComponent);
    }

    for (int node = 0; node < numNodes; ++node) {
        if (isArticulationPoint[node]) {
            result.articulationPoints.push_back(node);
        }
    }

    return result;
}