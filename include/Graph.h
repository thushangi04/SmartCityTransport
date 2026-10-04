#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>

#include "Journey.h"
#include "Locations.h"

struct Edge {
    int destination = -1;
    double distance = 0.0;
    double travelTime = 0.0;
    std::string transportType;
    std::string routeID;
};

enum class WeightMode {
    DISTANCE,
    TIME
};

class Graph {
private:
    std::vector<std::vector<Edge>> adjacencyList;

public:
    Graph();

    void addEdge(
        int source,
        int destination,
        double distance,
        double travelTime,
        const std::string& transportType,
        const std::string& routeID);

    const std::vector<Edge>& neighbours(int location) const;

    void displayGraph() const;

    bool bfs(
        int source,
        int destination,
        std::vector<int>* traversal = nullptr) const;

    /*
     * Traditional Dijkstra search using either pure distance
     * or pure in-vehicle travel time as the edge weight.
     */
    std::vector<JourneySegment> getShortestPath(
        int source,
        int destination,
        WeightMode mode = WeightMode::TIME) const;

    /*
     * Timetable-aware Dijkstra search.
     *
     * This minimizes estimated door-to-door public transport
     * time using:
     *   - in-vehicle travel time
     *   - scheduled waiting time
     *   - a fixed interchange/transfer penalty
     *
     * Vehicle-capacity congestion is handled later by the
     * journey simulator and is therefore not known while the
     * graph route is being selected.
     */
    std::vector<JourneySegment> getFastestPath(
        int source,
        int destination,
        const std::string& departureTime) const;

    /*
     * Print a normal route summary.
     */
    void printPath(
        const std::vector<JourneySegment>& path) const;

    /*
     * Print a timetable-aware route summary including
     * scheduled waiting and transfer time.
     */
    void printPath(
        const std::vector<JourneySegment>& path,
        const std::string& departureTime) const;
};

#endif
