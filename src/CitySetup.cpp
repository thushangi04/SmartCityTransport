#include "CitySetup.h"

namespace {
    double busTime(double distanceKm) {
        return (distanceKm / 30.0) * 60.0;
    }

    double trainTime(double distanceKm) {
        return (distanceKm / 60.0) * 60.0;
    }
}

void buildNovaCityGraph(Graph& graph) {
    // B01 - Northern Line
    graph.addEdge(NORTH_RESIDENTIAL, UNIVERSITY, 3, busTime(3), "Bus", "B01");
    graph.addEdge(UNIVERSITY, GENERAL_HOSPITAL, 3, busTime(3), "Bus", "B01");
    graph.addEdge(GENERAL_HOSPITAL, CENTRAL_STATION, 4, busTime(4), "Bus", "B01");
    graph.addEdge(CENTRAL_STATION, SHOPPING_MALL, 3, busTime(3), "Bus", "B01");

    // B02 - Southern Line
    graph.addEdge(CENTRAL_STATION, BUSINESS_DISTRICT, 5, busTime(5), "Bus", "B02");
    graph.addEdge(BUSINESS_DISTRICT, SOUTH_RESIDENTIAL, 4, busTime(4), "Bus", "B02");
    graph.addEdge(SOUTH_RESIDENTIAL, INDUSTRIAL_ZONE, 5, busTime(5), "Bus", "B02");
    graph.addEdge(INDUSTRIAL_ZONE, SPORTS_COMPLEX, 4, busTime(4), "Bus", "B02");

    // B03 - Cross-City Line
    graph.addEdge(NORTH_RESIDENTIAL, GENERAL_HOSPITAL, 5, busTime(5), "Bus", "B03");
    graph.addEdge(GENERAL_HOSPITAL, CENTRAL_STATION, 4, busTime(4), "Bus", "B03");
    graph.addEdge(CENTRAL_STATION, SHOPPING_MALL, 3, busTime(3), "Bus", "B03");
    graph.addEdge(SHOPPING_MALL, SPORTS_COMPLEX, 6, busTime(6), "Bus", "B03");

    // T01 - Main Railway Line
    graph.addEdge(CENTRAL_STATION, BUSINESS_DISTRICT, 6, trainTime(6), "Train", "T01");
    graph.addEdge(BUSINESS_DISTRICT, AIRPORT, 8, trainTime(8), "Train", "T01");

    // T02 - Airport Express
    graph.addEdge(CENTRAL_STATION, AIRPORT, 11, trainTime(11), "Train", "T02");
}

std::vector<Bus> createDefaultBuses() {
    return {
        Bus("BUS01", "B01"),
        Bus("BUS02", "B02"),
        Bus("BUS03", "B03")
    };
}

std::vector<Train> createDefaultTrains() {
    return {
        Train("TRAIN01", "T01"),
        Train("TRAIN02", "T02")
    };
}
