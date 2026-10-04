#include "NetworkAnalyzer.h"
#include "Locations.h"
#include <algorithm>
#include <climits>
#include <iomanip>
#include <iostream>
#include <set>

namespace {
    template <typename K>
    K keyWithHighestValue(const std::map<K, int>& values, K fallback) {
        K best = fallback;
        int bestValue = -1;
        for (const auto& [key, value] : values) {
            if (value > bestValue) {
                bestValue = value;
                best = key;
            }
        }
        return best;
    }
}

AnalysisResult NetworkAnalyzer::analyze(
    const std::vector<JourneyRecord>& records,
    const std::vector<RouteServiceState>& serviceStats,
    const std::vector<DemandPeriod>& demandPeriods) const {

    AnalysisResult result;
    result.metrics.generatedPassengers = static_cast<int>(records.size());

    double totalJourneyTime = 0.0;
    double totalWaitingTime = 0.0;
    double totalInterchangeTime = 0.0;
    double totalDistance = 0.0;
    int totalTransfers = 0;

    for (const JourneyRecord& record : records) {
        result.sourceUsage[record.source]++;
        result.destinationUsage[record.destination]++;

        if (!record.completed) {
            ++result.metrics.failedJourneys;
            continue;
        }

        ++result.metrics.completedJourneys;
        totalJourneyTime += record.totalJourneyTime;
        totalWaitingTime += record.waitingTime;
        totalInterchangeTime += record.interchangeTime;
        totalDistance += record.totalDistance;
        totalTransfers += record.transfers;

        for (const std::string& route : record.routesUsed) {
            result.routeUsage[route]++;
        }

        std::set<int> uniqueStations(record.visitedLocations.begin(),
                                     record.visitedLocations.end());
        for (int station : uniqueStations) {
            result.stationUsage[station]++;
        }

        for (int station : record.transferLocations) {
            result.transferUsage[station]++;
        }
    }

    if (result.metrics.completedJourneys > 0) {
        const double completed = static_cast<double>(result.metrics.completedJourneys);
        result.metrics.averageJourneyTime = totalJourneyTime / completed;
        result.metrics.averageWaitingTime = totalWaitingTime / completed;
        result.metrics.averageInterchangeTime = totalInterchangeTime / completed;
        result.metrics.averageDistance = totalDistance / completed;
        result.metrics.averageTransfers = static_cast<double>(totalTransfers) / completed;
    }

    if (result.metrics.generatedPassengers > 0) {
        result.metrics.successRate =
            (static_cast<double>(result.metrics.completedJourneys) /
             result.metrics.generatedPassengers) * 100.0;
    }

    int busAssigned = 0;
    int busCapacity = 0;
    int trainAssigned = 0;
    int trainCapacity = 0;

    for (const RouteServiceState& state : serviceStats) {
        const int totalCapacity = state.capacityPerDeparture * state.departures;
        result.metrics.maximumQueueLength =
            std::max(result.metrics.maximumQueueLength, state.maxQueueLength);
        result.metrics.passengersLeftWaiting += state.rejectedBoardings;

        if (state.transportType == "Bus") {
            busAssigned += state.assignedBoardings;
            busCapacity += totalCapacity;
        }
        else if (state.transportType == "Train") {
            trainAssigned += state.assignedBoardings;
            trainCapacity += totalCapacity;
        }
    }

    if (busCapacity > 0) {
        result.metrics.averageBusUtilization =
            (static_cast<double>(busAssigned) / busCapacity) * 100.0;
    }
    if (trainCapacity > 0) {
        result.metrics.averageTrainUtilization =
            (static_cast<double>(trainAssigned) / trainCapacity) * 100.0;
    }

    int highestBusUsage = -1;
    int highestTrainUsage = -1;
    for (const auto& [route, count] : result.routeUsage) {
        if (!route.empty() && route[0] == 'B' && count > highestBusUsage) {
            highestBusUsage = count;
            result.busiestBusRoute = route;
        }
        if (!route.empty() && route[0] == 'T' && count > highestTrainUsage) {
            highestTrainUsage = count;
            result.busiestTrainRoute = route;
        }
    }

    if (!result.stationUsage.empty()) {
        result.busiestStation = keyWithHighestValue(result.stationUsage, -1);

        int minUsage = INT_MAX;
        for (int station = 0; station < NUM_LOCATIONS; ++station) {
            const int usage = result.stationUsage.count(station)
                ? result.stationUsage.at(station) : 0;
            if (usage < minUsage) {
                minUsage = usage;
                result.leastUsedStation = station;
            }
        }
    }

    if (!result.transferUsage.empty()) {
        result.busiestTransferStation = keyWithHighestValue(result.transferUsage, -1);
    }

    int maxDemand = -1;
    for (const DemandPeriod& period : demandPeriods) {
        if (period.passengerCount > maxDemand) {
            maxDemand = period.passengerCount;
            result.busiestPeriod = period.periodName;
        }
    }

    return result;
}

void NetworkAnalyzer::printReport(
    const AnalysisResult& result,
    const std::vector<DemandPeriod>& demandPeriods) const {

    const PerformanceMetrics& m = result.metrics;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n============================================================\n"
              << "          NOVA CITY TRANSPORTATION SYSTEM REPORT\n"
              << "============================================================\n"
              << "\nSIMULATION SUMMARY\n"
              << "------------------------------------------------------------\n"
              << "Passengers Generated        : " << m.generatedPassengers << '\n'
              << "Completed Journeys          : " << m.completedJourneys << '\n'
              << "Failed Journeys             : " << m.failedJourneys << '\n'
              << "Journey Success Rate        : " << m.successRate << "%\n";

    std::cout << "\nPASSENGER DEMAND\n"
              << "------------------------------------------------------------\n";
    for (const DemandPeriod& period : demandPeriods) {
        std::cout << std::left << std::setw(28) << period.periodName
                  << ": " << period.passengerCount << '\n';
    }
    std::cout << "Highest Demand Period       : " << result.busiestPeriod << '\n';

    std::cout << "\nROUTE USAGE\n"
              << "------------------------------------------------------------\n";
    for (const auto& [route, count] : result.routeUsage) {
        std::cout << std::left << std::setw(28) << route
                  << ": " << count << " passenger journeys\n";
    }
    std::cout << "Busiest Bus Route           : "
              << (result.busiestBusRoute.empty() ? "N/A" : result.busiestBusRoute) << '\n'
              << "Busiest Train Route         : "
              << (result.busiestTrainRoute.empty() ? "N/A" : result.busiestTrainRoute) << '\n';

    std::cout << "\nSTATION / INTERCHANGE USAGE\n"
              << "------------------------------------------------------------\n"
              << "Busiest Location            : "
              << locationName(result.busiestStation) << '\n'
              << "Least Used Location         : "
              << locationName(result.leastUsedStation) << '\n'
              << "Most Used Interchange       : "
              << locationName(result.busiestTransferStation) << '\n';

    if (result.busiestTransferStation >= 0) {
        std::cout << "Transfers at Main Interchange: "
                  << result.transferUsage.at(result.busiestTransferStation) << '\n';
    }

    std::cout << "\nPERFORMANCE\n"
              << "------------------------------------------------------------\n"
              << "Average Journey Time        : " << m.averageJourneyTime << " min\n"
              << "Average Scheduled Waiting   : " << m.averageWaitingTime << " min\n"
              << "Average Interchange Time    : " << m.averageInterchangeTime << " min\n"
              << "Average Travel Distance     : " << m.averageDistance << " km\n"
              << "Average Transfers           : " << m.averageTransfers << '\n'
              << "Average Bus Utilization     : " << m.averageBusUtilization << "%\n"
              << "Average Train Utilization   : " << m.averageTrainUtilization << "%\n"
              << "Maximum Queue Length        : " << m.maximumQueueLength << '\n'
              << "Passengers Left Waiting     : " << m.passengersLeftWaiting << '\n';

    std::cout << "\nNETWORK OBSERVATIONS\n"
              << "------------------------------------------------------------\n"
              << "1. " << locationName(result.busiestStation)
              << " handles the highest passenger traffic.\n"
              << "2. " << (result.busiestBusRoute.empty() ? "No bus route" : result.busiestBusRoute)
              << " is the most heavily used bus service.\n"
              << "3. " << (result.busiestTrainRoute.empty() ? "No train route" : result.busiestTrainRoute)
              << " is the most heavily used train service.\n"
              << "4. " << result.busiestPeriod
              << " produces the highest simulated demand.\n"
              << "5. Efficiency should be judged using success rate, scheduled waiting,\n"
              << "   interchange time, transfers, utilization and congestion together rather than a\n"
              << "   single arbitrary percentage.\n"
              << "============================================================\n";
}
