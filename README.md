# GraphTransit

GraphTransit is a C++17 project I built to model a public transportation network as a graph and experiment with a range of classic graph algorithms on top of it. The core idea came from a simple question: how do you compute the minimum number of buses someone needs to board to get from one stop to another? A plain stop-to-stop graph can't answer that, because it has no concept of which bus you're currently on — so the project builds a transformed graph that encodes that information explicitly.

From there it grew into a small collection of related graph problems: fastest routes with real weights, critical stops/connections in the network, maximum passenger flow, alternative routes, and reachability under a transfer limit.

## Features

- Bus network modeling: stops, routes, and which buses serve which stops (a stop can be served by multiple buses).
- Dummy-bus-node graph transformation to encode "which bus am I on" as part of the graph structure.
- Minimum bus boarding via 0-1 BFS, with path reconstruction.
- Fastest route via Dijkstra, with path reconstruction.
- Critical infrastructure analysis (articulation points, bridges, connected components) via Tarjan's algorithm.
- Maximum flow via Dinic's algorithm, using a dedicated `FlowNetwork` class.
- K shortest simple paths via Yen's algorithm.
- Limited-transfer reachability via a layered state-space graph.
- A lightweight assertion-based test suite covering all of the above.
- Python scripts for generating sample networks, visualizing them, and benchmarking.

## Algorithms

### Graph transformation (dummy bus nodes)

Given N stops and M buses, `GraphTransformer` builds a graph with N + M nodes: the first N are the real stops, and the next M are "dummy" nodes, one per bus. For every stop a bus visits:

```
stop         -> dummyBusNode   (weight 1)   // boarding
dummyBusNode -> stop           (weight 0)   // riding, no extra cost
```

Because every edge weight is 0 or 1, the shortest path between two stops in this graph is exactly the minimum number of buses boarded, and it can be computed with 0-1 BFS instead of a general shortest-path algorithm.

### 0-1 BFS

Runs on the transformed graph above. Uses a deque instead of a priority queue since weights are only 0 or 1 — weight-0 edges get pushed to the front, weight-1 edges to the back. Also supports reconstructing the actual path (which stops and which dummy bus nodes were visited), not just the cost.

### Dijkstra

Standard implementation with a priority queue, used wherever edge weights are arbitrary non-negative numbers (for example, real travel times rather than boarding counts). Validates that weights aren't negative and reconstructs the path.

### Tarjan's algorithm

Finds articulation points, bridges, and connected components using DFS discovery/low-link values. This runs on a separate undirected graph representing direct stop-to-stop connectivity — it's a different graph than the dummy-bus-node one, since "critical stop" is an undirected connectivity question, not a boarding-cost question.

### Dinic's maximum flow

Implemented as its own `FlowNetwork` class rather than reusing `Graph`. Flow algorithms need mutable per-edge state (residual capacity, current flow, a paired reverse edge), which doesn't fit the fixed-weight `Edge` struct that `Graph` uses. Includes the residual graph, BFS level-graph construction, DFS blocking flow, and the current-arc optimization.

### Yen's K shortest paths

Finds up to K shortest simple (loopless) paths between two nodes, in increasing order of cost. Uses `Algorithms::dijkstra` as the subroutine for each candidate path and filters out duplicates.

### Limited transfer routing

Models the state `(stop, buses boarded so far)` as a layered graph — one full copy of the transformed graph per "layer," where boarding a bus moves you to the next layer and riding stays within the same layer. Checking whether a destination is reachable within K transfers means checking reachability to that stop across the first K+1 layers.

## Project structure

```
graph-transit/
├── include/
│   ├── Graph.hpp
│   ├── BusNetwork.hpp
│   ├── GraphTransformer.hpp
│   ├── Algorithms.hpp
│   ├── Tarjan.hpp
│   ├── FlowNetwork.hpp
│   └── RouteAlgorithms.hpp
├── src/
│   ├── main.cpp
│   ├── Graph.cpp
│   ├── BusNetwork.cpp
│   ├── GraphTransformer.cpp
│   ├── Algorithms.cpp
│   ├── Tarjan.cpp
│   ├── FlowNetwork.cpp
│   └── RouteAlgorithms.cpp
├── tests/
│   └── test_main.cpp
├── python/
│   ├── generate_network.py
│   ├── visualize.py
│   └── benchmark.py
├── data/
│   └── network.json
├── CMakeLists.txt
└── README.md
```

## Building

```
g++ -std=c++17 -I include src/main.cpp src/Graph.cpp src/BusNetwork.cpp src/GraphTransformer.cpp src/Algorithms.cpp src/Tarjan.cpp src/FlowNetwork.cpp src/RouteAlgorithms.cpp -o graph_transit
```

Run it (Windows):

```
.\graph_transit.exe
```

A `CMakeLists.txt` is also included if you'd rather use CMake instead of compiling manually.

## Testing

The test suite is a set of plain assertions in `tests/test_main.cpp` — no external testing framework. It covers every module: the graph itself, the bus network, the dummy-node transformation, 0-1 BFS, Dijkstra, Tarjan's algorithm, max flow, Yen's algorithm, and the layered transfer graph. All tests currently pass.

Build and run the tests:

```
g++ -std=c++17 -I include tests/test_main.cpp src/Graph.cpp src/BusNetwork.cpp src/GraphTransformer.cpp src/Algorithms.cpp src/Tarjan.cpp src/FlowNetwork.cpp src/RouteAlgorithms.cpp -o graph_transit_tests
```

```
.\graph_transit_tests.exe
```

## Python tools

A few helper scripts under `python/` for generating test data, visualizing it, and rough benchmarking. These are separate from the C++ codebase and just work against the JSON network format.

Generate a random network:

```
python python/generate_network.py --stops 50 --buses 20 --output data/network.json --seed 42
```

Visualization needs `networkx` and `matplotlib`:

```
python -m pip install networkx matplotlib
python python/visualize.py --input data/network.json
```

Run the benchmark script:

```
python python/benchmark.py --iterations 20
```

Example result from my machine:

```
Runs: 20
Minimum: 16.835 ms
Maximum: 31.872 ms
Average: 21.157 ms
Standard deviation: 3.288 ms
```

Keep in mind this measures full process execution time (startup, network generation/loading, etc.), not just the algorithm itself, so it shouldn't be read as an isolated performance measurement of, say, 0-1 BFS or Dijkstra on their own.

## Design notes

Two design decisions are probably worth explaining:

**Why dummy bus nodes instead of just weighting stop-to-stop edges?** A stop-to-stop graph loses information about which bus you're riding. If two different buses both go from stop A to stop B, a plain graph can't distinguish "I'm already on a bus that happens to stop at B" from "I need to board a new bus to get to B." Splitting the edge through a dummy bus node makes that distinction explicit and lets a simple 0-1 BFS produce the right answer.

**Why is `FlowNetwork` a separate class instead of extending `Graph`?** `Graph`'s `Edge` struct is a fixed `{to, weight}` pair, which works fine for algorithms that only read edge weights. Flow algorithms need to track capacity and current flow per edge and mutate them as flow is pushed and cancelled, plus maintain a paired reverse residual edge for each forward edge. Bolting that onto `Graph` would complicate it for every other algorithm that doesn't need it, so it lives in its own class instead.

## Future improvements

- Extend the layered transfer graph to support non-uniform transfer costs instead of just a transfer count cap.
- Add multi-criteria routing (combining travel time and transfer penalties), which was part of the original plan but isn't implemented yet.
- Add more systematic benchmarking that isolates individual algorithms rather than measuring whole-program runs.
- Expand test coverage with randomly generated networks checked against a brute-force reference, in addition to the current hand-written test cases.
