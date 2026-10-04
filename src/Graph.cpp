#include "Graph.h"
#include "TransportConfig.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <tuple>
#include <utility>

namespace {

int timeToMinutes(const std::string& time) {
    if (time.size() != 5 || time[2] != ':') {
        return -1;
    }

    try {
        const int hour = std::stoi(time.substr(0, 2));
        const int minute = std::stoi(time.substr(3, 2));

        if (hour < 0 || hour > 23 ||
            minute < 0 || minute > 59) {
            return -1;
        }

        return (hour * 60) + minute;
    }
    catch (...) {
        return -1;
    }
}

std::string minutesToClock(double absoluteMinutes) {
    int rounded = static_cast<int>(std::round(absoluteMinutes));

    const int dayOffset = rounded / (24 * 60);
    rounded %= (24 * 60);

    if (rounded < 0) {
        rounded += 24 * 60;
    }

    const int hour = rounded / 60;
    const int minute = rounded % 60;

    std::ostringstream out;
    out << std::setw(2) << std::setfill('0') << hour
        << ':'
        << std::setw(2) << std::setfill('0') << minute;

    if (dayOffset > 0) {
        out << " (+" << dayOffset << " day)";
    }

    return out.str();
}

double nextScheduledDeparture(
    double readyMinute,
    const std::string& transportType) {

    const double serviceStart =
        static_cast<double>(TransportConfig::SERVICE_START_MINUTE);

    const double serviceEnd =
        static_cast<double>(TransportConfig::SERVICE_END_MINUTE);

    const double headway =
        TransportConfig::headwayForTransport(transportType);

    if (readyMinute <= serviceStart) {
        return serviceStart;
    }

    const double minutesAfterStart =
        readyMinute - serviceStart;

    const int departureIndex =
        static_cast<int>(
            std::ceil((minutesAfterStart / headway) - 1e-9));

    const double departure =
        serviceStart + (departureIndex * headway);

    if (departure >= serviceEnd) {
        return -1.0;
    }

    return departure;
}

}

Graph::Graph()
    : adjacencyList(NUM_LOCATIONS) {}

void Graph::addEdge(
    int source,
    int destination,
    double distance,
    double travelTime,
    const std::string& transportType,
    const std::string& routeID) {

    if (source < 0 || source >= NUM_LOCATIONS ||
        destination < 0 || destination >= NUM_LOCATIONS) {
        return;
    }

    adjacencyList[source].push_back({
        destination,
        distance,
        travelTime,
        transportType,
        routeID
    });

    adjacencyList[destination].push_back({
        source,
        distance,
        travelTime,
        transportType,
        routeID
    });
}

const std::vector<Edge>& Graph::neighbours(int location) const {
    return adjacencyList.at(location);
}

void Graph::displayGraph() const {
    std::cout
        << "\n================ NOVA CITY NETWORK ================\n";

    for (int i = 0; i < NUM_LOCATIONS; ++i) {
        std::cout
            << "\n[" << i << "] "
            << locationName(i)
            << "\n";

        for (const Edge& edge : adjacencyList[i]) {
            std::cout
                << "   -> "
                << locationName(edge.destination)
                << " | " << edge.routeID
                << " | " << edge.transportType
                << " | " << edge.distance << " km"
                << " | " << edge.travelTime << " min\n";
        }
    }
}

bool Graph::bfs(
    int source,
    int destination,
    std::vector<int>* traversal) const {

    if (source < 0 || source >= NUM_LOCATIONS ||
        destination < 0 || destination >= NUM_LOCATIONS) {
        return false;
    }

    std::vector<bool> visited(NUM_LOCATIONS, false);
    std::queue<int> q;

    visited[source] = true;
    q.push(source);

    while (!q.empty()) {
        const int current = q.front();
        q.pop();

        if (traversal != nullptr) {
            traversal->push_back(current);
        }

        if (current == destination) {
            return true;
        }

        for (const Edge& edge : adjacencyList[current]) {
            if (!visited[edge.destination]) {
                visited[edge.destination] = true;
                q.push(edge.destination);
            }
        }
    }

    return false;
}

std::vector<JourneySegment> Graph::getShortestPath(
    int source,
    int destination,
    WeightMode mode) const {

    std::vector<JourneySegment> emptyPath;

    if (source < 0 || source >= NUM_LOCATIONS ||
        destination < 0 || destination >= NUM_LOCATIONS ||
        source == destination) {
        return emptyPath;
    }

    const double INF =
        std::numeric_limits<double>::infinity();

    std::vector<double> distance(
        NUM_LOCATIONS,
        INF);

    std::vector<int> previous(
        NUM_LOCATIONS,
        -1);

    std::vector<Edge> previousEdge(
        NUM_LOCATIONS);

    std::vector<bool> hasPreviousEdge(
        NUM_LOCATIONS,
        false);

    using State = std::pair<double, int>;

    std::priority_queue<
        State,
        std::vector<State>,
        std::greater<State>> pq;

    distance[source] = 0.0;
    pq.push({0.0, source});

    while (!pq.empty()) {
        const auto [currentCost, current] = pq.top();
        pq.pop();

        if (currentCost > distance[current]) {
            continue;
        }

        if (current == destination) {
            break;
        }

        for (const Edge& edge : adjacencyList[current]) {
            const double weight =
                (mode == WeightMode::DISTANCE)
                    ? edge.distance
                    : edge.travelTime;

            const double newCost =
                distance[current] + weight;

            if (newCost < distance[edge.destination]) {
                distance[edge.destination] = newCost;
                previous[edge.destination] = current;
                previousEdge[edge.destination] = edge;
                hasPreviousEdge[edge.destination] = true;

                pq.push({
                    newCost,
                    edge.destination
                });
            }
        }
    }

    if (distance[destination] == INF) {
        return emptyPath;
    }

    std::vector<JourneySegment> reversedPath;
    int current = destination;

    while (current != source) {
        if (previous[current] == -1 ||
            !hasPreviousEdge[current]) {
            return emptyPath;
        }

        const Edge& edge =
            previousEdge[current];

        reversedPath.push_back({
            previous[current],
            current,
            edge.routeID,
            edge.transportType,
            edge.distance,
            edge.travelTime
        });

        current = previous[current];
    }

    std::reverse(
        reversedPath.begin(),
        reversedPath.end());

    return reversedPath;
}

std::vector<JourneySegment> Graph::getFastestPath(
    int source,
    int destination,
    const std::string& departureTime) const {

    std::vector<JourneySegment> emptyPath;

    if (source < 0 || source >= NUM_LOCATIONS ||
        destination < 0 || destination >= NUM_LOCATIONS ||
        source == destination) {
        return emptyPath;
    }

    const int startMinute =
        timeToMinutes(departureTime);

    if (startMinute < 0) {
        return emptyPath;
    }

    /*
     * A location alone is not enough for this Dijkstra search.
     *
     * The cost of the next edge depends on whether the passenger
     * stays on the same service or changes to another route.
     * Therefore a state consists of:
     *
     *     (current location, current route)
     *
     * An empty route means the passenger has not boarded yet.
     */
    using RouteState =
        std::pair<int, std::string>;

    using QueueState =
        std::tuple<double, int, std::string>;

    const double INF =
        std::numeric_limits<double>::infinity();

    std::map<RouteState, double> earliestArrival;
    std::map<RouteState, RouteState> previousState;
    std::map<RouteState, Edge> previousEdge;

    std::priority_queue<
        QueueState,
        std::vector<QueueState>,
        std::greater<QueueState>> pq;

    const RouteState startState = {
        source,
        ""
    };

    earliestArrival[startState] =
        static_cast<double>(startMinute);

    pq.push({
        static_cast<double>(startMinute),
        source,
        ""
    });

    bool destinationFound = false;
    RouteState finalState;

    while (!pq.empty()) {
        const auto [
            currentMinute,
            currentLocation,
            currentRoute] = pq.top();

        pq.pop();

        const RouteState currentState = {
            currentLocation,
            currentRoute
        };

        const auto knownIt =
            earliestArrival.find(currentState);

        if (knownIt == earliestArrival.end() ||
            currentMinute > knownIt->second + 1e-9) {
            continue;
        }

        if (currentLocation == destination) {
            destinationFound = true;
            finalState = currentState;
            break;
        }

        for (const Edge& edge :
             adjacencyList[currentLocation]) {

            double departureMinute =
                currentMinute;

            /*
             * Staying on the same route means the passenger
             * remains on the same bus/train. There is no new
             * boarding wait and no transfer penalty.
             */
            if (currentRoute != edge.routeID) {

                double readyMinute =
                    currentMinute;

                /*
                 * Changing from one route to another requires
                 * interchange/walking time before the passenger
                 * is ready to board the next service.
                 */
                if (!currentRoute.empty()) {
                    readyMinute +=
                        TransportConfig::TRANSFER_PENALTY_MINUTES;
                }

                departureMinute =
                    nextScheduledDeparture(
                        readyMinute,
                        edge.transportType);

                /*
                 * No scheduled service remains for this edge.
                 */
                if (departureMinute < 0.0) {
                    continue;
                }
            }

            const double arrivalMinute =
                departureMinute + edge.travelTime;

            const RouteState nextState = {
                edge.destination,
                edge.routeID
            };

            const auto nextIt =
                earliestArrival.find(nextState);

            const double knownArrival =
                (nextIt == earliestArrival.end())
                    ? INF
                    : nextIt->second;

            if (arrivalMinute + 1e-9 < knownArrival) {
                earliestArrival[nextState] =
                    arrivalMinute;

                previousState[nextState] =
                    currentState;

                previousEdge[nextState] =
                    edge;

                pq.push({
                    arrivalMinute,
                    edge.destination,
                    edge.routeID
                });
            }
        }
    }

    if (!destinationFound) {
        return emptyPath;
    }

    std::vector<JourneySegment> reversedPath;
    RouteState currentState = finalState;

    while (currentState != startState) {
        const auto stateIt =
            previousState.find(currentState);

        const auto edgeIt =
            previousEdge.find(currentState);

        if (stateIt == previousState.end() ||
            edgeIt == previousEdge.end()) {
            return emptyPath;
        }

        const RouteState& previous =
            stateIt->second;

        const Edge& edge =
            edgeIt->second;

        reversedPath.push_back({
            previous.first,
            currentState.first,
            edge.routeID,
            edge.transportType,
            edge.distance,
            edge.travelTime
        });

        currentState = previous;
    }

    std::reverse(
        reversedPath.begin(),
        reversedPath.end());

    return reversedPath;
}

void Graph::printPath(
    const std::vector<JourneySegment>& path) const {

    if (path.empty()) {
        std::cout
            << "No path available.\n";
        return;
    }

    double totalDistance = 0.0;
    double totalTime = 0.0;
    int transfers = 0;
    std::string previousRoute;

    std::cout
        << "\n================ ROUTE ================\n"
        << locationName(path.front().from)
        << '\n';

    for (const JourneySegment& segment : path) {
        if (!previousRoute.empty() &&
            previousRoute != segment.routeID) {
            ++transfers;
        }

        std::cout
            << "   |\n"
            << "   | " << segment.routeID
            << " - " << segment.transportType
            << " - " << segment.distance << " km"
            << " - " << segment.travelTime << " min\n"
            << "   v\n"
            << locationName(segment.to)
            << '\n';

        totalDistance +=
            segment.distance;

        totalTime +=
            segment.travelTime;

        previousRoute =
            segment.routeID;
    }

    std::cout
        << "---------------------------------------\n"
        << "Total Distance : "
        << totalDistance
        << " km\n"
        << "In-Vehicle Time: "
        << totalTime
        << " min\n"
        << "Route Transfers: "
        << transfers
        << '\n';
}

void Graph::printPath(
    const std::vector<JourneySegment>& path,
    const std::string& departureTime) const {

    if (path.empty()) {
        std::cout
            << "No path available.\n";
        return;
    }

    const int startMinute =
        timeToMinutes(departureTime);

    if (startMinute < 0) {
        std::cout
            << "Invalid departure time.\n";
        return;
    }

    double currentMinute =
        static_cast<double>(startMinute);

    double totalDistance = 0.0;
    double inVehicleTime = 0.0;
    double scheduledWaitingTime = 0.0;
    double transferPenaltyTime = 0.0;

    int transfers = 0;
    std::string previousRoute;

    std::cout
        << "\n=========== TIMETABLE-AWARE FASTEST ROUTE ===========\n"
        << "Requested departure: "
        << departureTime
        << "\n\n"
        << locationName(path.front().from)
        << '\n';

    for (const JourneySegment& segment : path) {

        if (previousRoute != segment.routeID) {

            if (!previousRoute.empty()) {
                ++transfers;

                currentMinute +=
                    TransportConfig::TRANSFER_PENALTY_MINUTES;

                transferPenaltyTime +=
                    TransportConfig::TRANSFER_PENALTY_MINUTES;

                std::cout
                    << "   |\n"
                    << "   | Interchange: "
                    << TransportConfig::TRANSFER_PENALTY_MINUTES
                    << " min\n";
            }

            const double departureMinute =
                nextScheduledDeparture(
                    currentMinute,
                    segment.transportType);

            if (departureMinute < 0.0) {
                std::cout
                    << "No scheduled service remains.\n";
                return;
            }

            const double wait =
                std::max(
                    0.0,
                    departureMinute - currentMinute);

            scheduledWaitingTime +=
                wait;

            currentMinute =
                departureMinute;

            std::cout
                << "   |\n"
                << "   | Board "
                << segment.routeID
                << " ("
                << segment.transportType
                << ") at "
                << minutesToClock(departureMinute)
                << " | wait "
                << wait
                << " min\n";
        }

        std::cout
            << "   | "
            << segment.distance
            << " km | "
            << segment.travelTime
            << " min travel\n"
            << "   v\n"
            << locationName(segment.to)
            << '\n';

        totalDistance +=
            segment.distance;

        inVehicleTime +=
            segment.travelTime;

        currentMinute +=
            segment.travelTime;

        previousRoute =
            segment.routeID;
    }

    const double estimatedTotalTime =
        currentMinute - startMinute;

    std::cout
        << "------------------------------------------------------\n"
        << "Arrival Time       : "
        << minutesToClock(currentMinute)
        << "\n"
        << "Total Distance     : "
        << totalDistance
        << " km\n"
        << "In-Vehicle Time    : "
        << inVehicleTime
        << " min\n"
        << "Scheduled Waiting  : "
        << scheduledWaitingTime
        << " min\n"
        << "Interchange Time   : "
        << transferPenaltyTime
        << " min\n"
        << "Estimated Total    : "
        << estimatedTotalTime
        << " min\n"
        << "Route Transfers    : "
        << transfers
        << '\n';
}
