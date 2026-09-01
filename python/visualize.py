import argparse
import json

import matplotlib.pyplot as plt
import networkx as nx


def load_network(filename):
    with open(filename, "r", encoding="utf-8") as file:
        return json.load(file)


def build_graph(network):
    graph = nx.Graph()

    num_stops = network["num_stops"]

    for stop in range(num_stops):
        graph.add_node(stop)

    for route in network["routes"]:
        stops = route["stops"]

        for i in range(len(stops) - 1):
            graph.add_edge(
                stops[i],
                stops[i + 1],
                bus=route["bus_id"]
            )

    return graph


def visualize_network(network):
    graph = build_graph(network)

    plt.figure(figsize=(12, 8))

    position = nx.spring_layout(
        graph,
        seed=42
    )

    nx.draw_networkx_nodes(
        graph,
        position,
        node_size=500
    )

    nx.draw_networkx_edges(
        graph,
        position,
        width=1.5
    )

    nx.draw_networkx_labels(
        graph,
        position,
        font_size=9
    )

    plt.title(
        f"GraphTransit Network | "
        f"{network['num_stops']} Stops | "
        f"{network['num_buses']} Buses"
    )

    plt.axis("off")
    plt.tight_layout()
    plt.show()


def main():
    parser = argparse.ArgumentParser(
        description="Visualize a GraphTransit bus network."
    )

    parser.add_argument(
        "--input",
        default="../data/network.json",
        help="Input network JSON file."
    )

    args = parser.parse_args()

    network = load_network(args.input)

    visualize_network(network)


if __name__ == "__main__":
    main()