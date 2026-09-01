import argparse
import json
import random


def generate_network(num_stops, num_buses, min_route_length, max_route_length, seed=None):
    if seed is not None:
        random.seed(seed)

    if num_stops <= 0:
        raise ValueError("Number of stops must be positive.")

    if num_buses <= 0:
        raise ValueError("Number of buses must be positive.")

    if min_route_length < 2:
        raise ValueError("Minimum route length must be at least 2.")

    if max_route_length > num_stops:
        max_route_length = num_stops

    if min_route_length > max_route_length:
        raise ValueError("Minimum route length cannot exceed maximum route length.")

    routes = []

    for bus_id in range(num_buses):
        route_length = random.randint(min_route_length, max_route_length)

        stops = random.sample(
            range(num_stops),
            route_length
        )

        routes.append({
            "bus_id": bus_id,
            "stops": stops
        })

    return {
        "num_stops": num_stops,
        "num_buses": num_buses,
        "routes": routes
    }


def main():
    parser = argparse.ArgumentParser(
        description="Generate a random bus transportation network."
    )

    parser.add_argument(
        "--stops",
        type=int,
        default=20,
        help="Number of stops."
    )

    parser.add_argument(
        "--buses",
        type=int,
        default=10,
        help="Number of buses."
    )

    parser.add_argument(
        "--min-route-length",
        type=int,
        default=3,
        help="Minimum number of stops in a bus route."
    )

    parser.add_argument(
        "--max-route-length",
        type=int,
        default=8,
        help="Maximum number of stops in a bus route."
    )

    parser.add_argument(
        "--output",
        default="../data/network.json",
        help="Output JSON file."
    )

    parser.add_argument(
        "--seed",
        type=int,
        default=None,
        help="Random seed for reproducibility."
    )

    args = parser.parse_args()

    network = generate_network(
        args.stops,
        args.buses,
        args.min_route_length,
        args.max_route_length,
        args.seed
    )

    with open(args.output, "w", encoding="utf-8") as file:
        json.dump(network, file, indent=4)

    print("Network generated successfully.")
    print(f"Stops: {network['num_stops']}")
    print(f"Buses: {network['num_buses']}")
    print(f"Saved to: {args.output}")


if __name__ == "__main__":
    main()