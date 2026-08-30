#pragma once

#include <vector>

// Represents the raw transportation network before graph transformation.
//
// Stores information in both directions:
// Bus -> Stops
// Stop -> Buses
//
// Graph transformations, dummy nodes, and algorithms are handled by
// separate components.
class BusNetwork {
public:
    // Constructs a network with stop IDs from 0 to numStops - 1.
    explicit BusNetwork(int numStops);

    // Adds a bus route containing an ordered sequence of stop IDs.
    // The bus ID is assigned automatically based on insertion order.
    void addBusRoute(const std::vector<int>& stops);

    // Returns the ordered list of stops visited by a bus.
    const std::vector<int>& stopsForBus(int busId) const;

    // Returns all buses that visit a particular stop.
    const std::vector<int>& busesForStop(int stopId) const;

    // Returns the total number of stops.
    int numStops() const;

    // Returns the total number of buses.
    int numBuses() const;

private:
    int numStops_;

    // Indexed by bus ID.
    // Each entry contains the ordered stops visited by that bus.
    std::vector<std::vector<int>> busRoutes_;

    // Indexed by stop ID.
    // Each entry contains the buses visiting that stop.
    std::vector<std::vector<int>> stopToBuses_;
};