#include "BusNetwork.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace {

// Validates a stop ID.
void validateStop(int stopId, int numStops) {
    if (stopId < 0 || stopId >= numStops) {
        throw std::out_of_range(
            "BusNetwork: stop id " + std::to_string(stopId) +
            " is out of range for a network with " +
            std::to_string(numStops) + " stops"
        );
    }
}

// Validates a bus ID.
void validateBus(int busId, int numBuses) {
    if (busId < 0 || busId >= numBuses) {
        throw std::out_of_range(
            "BusNetwork: bus id " + std::to_string(busId) +
            " is out of range for a network with " +
            std::to_string(numBuses) + " buses"
        );
    }
}

} // namespace

BusNetwork::BusNetwork(int numStops) : numStops_(numStops) {
    if (numStops < 0) {
        throw std::invalid_argument(
            "BusNetwork: number of stops cannot be negative"
        );
    }

    stopToBuses_.resize(numStops);
}

void BusNetwork::addBusRoute(const std::vector<int>& stops) {
    // Validate all stop IDs before modifying the network.
    for (int stopId : stops) {
        validateStop(stopId, numStops_);
    }

    // The next route index becomes the new bus ID.
    int busId = numBuses();

    // Store the ordered route.
    busRoutes_.push_back(stops);

    // Update Stop -> Buses mapping.
    // Avoid adding the same bus multiple times if a stop appears
    // more than once in the same route.
    for (int stopId : stops) {
        std::vector<int>& buses = stopToBuses_[stopId];

        if (std::find(buses.begin(), buses.end(), busId) == buses.end()) {
            buses.push_back(busId);
        }
    }
}

const std::vector<int>& BusNetwork::stopsForBus(int busId) const {
    validateBus(busId, numBuses());

    return busRoutes_[busId];
}

const std::vector<int>& BusNetwork::busesForStop(int stopId) const {
    validateStop(stopId, numStops_);

    return stopToBuses_[stopId];
}

int BusNetwork::numStops() const {
    return numStops_;
}

int BusNetwork::numBuses() const {
    return static_cast<int>(busRoutes_.size());
}