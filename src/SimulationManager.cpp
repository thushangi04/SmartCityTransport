#include "SimulationManager.h"
#include "CitySetup.h"
#include "Locations.h"
#include "TransportConfig.h"

#include <algorithm>
#include <cmath>
#include <iostream>

SimulationManager::SimulationManager(
    Graph& graphRef,
    unsigned int seed)

    : graph(graphRef),
      buses(createDefaultBuses()),
      trains(createDefaultTrains()),
      demandGenerator(seed),
      journeySimulator(buses, trains),
      demandPeriods({
          {"Early Morning", 5, 7, DemandLevel::LOW, 30},
          {"Morning Peak", 7, 9, DemandLevel::VERY_HIGH, 150},
          {"Mid-Morning", 9, 12, DemandLevel::MEDIUM, 70},
          {"Midday", 12, 14, DemandLevel::HIGH, 100},
          {"Afternoon", 14, 16, DemandLevel::MEDIUM, 60},
          {"Evening Peak", 16, 19, DemandLevel::VERY_HIGH, 170},
          {"Evening", 19, 22, DemandLevel::MEDIUM, 80},
          {"Late Night", 22, 24, DemandLevel::LOW, 20}
      }) {}

std::unordered_map<std::string, RouteServiceState>
SimulationManager::createServiceStates() const {

    std::unordered_map<std::string, RouteServiceState> states;

    const int operatingMinutes =
        TransportConfig::SERVICE_END_MINUTE
        - TransportConfig::SERVICE_START_MINUTE;

    for (const Bus& bus : buses) {
        const int departures =
            std::max(
                1,
                static_cast<int>(
                    std::ceil(
                        operatingMinutes
                        / bus.getHeadwayMinutes())));

        RouteServiceState state;
        state.periodName = "Operating Day";
        state.routeID = bus.getRouteID();
        state.transportType = bus.getType();
        state.capacityPerDeparture = bus.getCapacity();
        state.departures = departures;
        state.headwayMinutes = bus.getHeadwayMinutes();
        state.serviceStartMinute =
            TransportConfig::SERVICE_START_MINUTE;
        state.serviceEndMinute =
            TransportConfig::SERVICE_END_MINUTE;
        state.boardedPerDeparture.assign(
            departures,
            0);
        state.queuedForDeparture.assign(
            departures,
            0);

        states[bus.getRouteID()] = state;
    }

    for (const Train& train : trains) {
        const int departures =
            std::max(
                1,
                static_cast<int>(
                    std::ceil(
                        operatingMinutes
                        / train.getHeadwayMinutes())));

        RouteServiceState state;
        state.periodName = "Operating Day";
        state.routeID = train.getRouteID();
        state.transportType = train.getType();
        state.capacityPerDeparture = train.getCapacity();
        state.departures = departures;
        state.headwayMinutes = train.getHeadwayMinutes();
        state.serviceStartMinute =
            TransportConfig::SERVICE_START_MINUTE;
        state.serviceEndMinute =
            TransportConfig::SERVICE_END_MINUTE;
        state.boardedPerDeparture.assign(
            departures,
            0);
        state.queuedForDeparture.assign(
            departures,
            0);

        states[train.getRouteID()] = state;
    }

    return states;
}

JourneyRecord SimulationManager::makeRecord(
    const Passenger& passenger,
    const Journey& journey,
    const DemandPeriod& period) const {

    JourneyRecord record;

    record.passengerID =
        passenger.getPassengerID();

    record.periodName =
        period.periodName;

    record.source =
        passenger.getSource();

    record.destination =
        passenger.getDestination();

    record.departureTime =
        passenger.getDepartureTime();

    record.totalDistance =
        journey.totalDistance;

    record.travelTime =
        journey.totalTravelTime;

    record.waitingTime =
        journey.totalWaitingTime;

    record.interchangeTime =
        journey.totalInterchangeTime;

    record.totalJourneyTime =
        journey.totalTravelTime
        + journey.totalWaitingTime
        + journey.totalInterchangeTime;

    record.transfers =
        journey.transfers;

    record.routesUsed =
        journey.routesUsed;

    record.visitedLocations =
        journey.visitedLocations;

    record.transferLocations =
        journey.transferLocations;

    record.completed =
        journey.completed;

    record.failureReason =
        journey.failureReason;

    return record;
}

void SimulationManager::simulatePeriod(
    const DemandPeriod& period,
    int samplePrintLimit,
    std::unordered_map<
        std::string,
        RouteServiceState>& serviceStates) {

    std::cout
        << "\n===== "
        << period.periodName
        << " ("
        << period.startHour
        << ":00-"
        << period.endHour
        << ":00) =====\n";

    std::cout
        << "Passengers generated: "
        << period.passengerCount
        << '\n';

    std::vector<Passenger> passengers =
        demandGenerator.generatePassengers(period);

    /*
     * FIX 1:
     * Process passengers in chronological order.
     */
    std::stable_sort(
        passengers.begin(),
        passengers.end(),
        [](
            const Passenger& first,
            const Passenger& second) {

            return first.getDepartureTime()
                < second.getDepartureTime();
        });

    int samplesPrinted = 0;
    int completed = 0;
    int failed = 0;

    for (Passenger& passenger : passengers) {

        /*
         * FIX 5:
         * Use timetable-aware Dijkstra routing.
         *
         * The route now accounts for:
         * - scheduled waiting time
         * - in-vehicle travel time
         * - transfer/interchange time
         *
         * Capacity congestion is still applied by
         * JourneySimulator after the path is chosen.
         */
        std::vector<JourneySegment> route =
            graph.getFastestPath(
                passenger.getSource(),
                passenger.getDestination(),
                passenger.getDepartureTime());

        Journey journey;

        const bool verbose =
            samplesPrinted < samplePrintLimit;

        if (route.empty()) {
            journey.failureReason =
                "No timetable-feasible graph path between source and destination";
        }
        else {
            journey =
                journeySimulator.simulateJourney(
                    passenger,
                    route,
                    serviceStates,
                    verbose);
        }

        statisticsManager.addRecord(
            makeRecord(
                passenger,
                journey,
                period));

        if (journey.completed) {
            ++completed;
        }
        else {
            ++failed;
        }

        if (verbose) {
            std::cout
                << "  Result: "
                << (
                    journey.completed
                        ? "Completed"
                        : "Failed")
                << " | "
                << locationName(
                    passenger.getSource())
                << " -> "
                << locationName(
                    passenger.getDestination())
                << " | distance "
                << journey.totalDistance
                << " km"
                << " | total time "
                << (
                    journey.totalTravelTime
                    + journey.totalWaitingTime
                    + journey.totalInterchangeTime)
                << " min\n";

            if (!journey.completed) {
                std::cout
                    << "  Failure: "
                    << journey.failureReason
                    << '\n';
            }

            ++samplesPrinted;
        }
    }

    std::cout
        << "Period completed: "
        << completed
        << " successful, "
        << failed
        << " failed.\n";
}

void SimulationManager::runOperatingDaySimulation(
    int samplePrintLimitPerPeriod) {

    statisticsManager.clear();
    allServiceStats.clear();

    /*
     * FIX 3:
     * Reset passenger IDs and random generator so every
     * run with the same seed is reproducible.
     */
    demandGenerator.reset();

    /*
     * FIX 2:
     * Create one continuous operating-day timetable.
     */
    auto serviceStates =
        createServiceStates();

    std::cout
        << "\n============================================================\n"
        << "      STARTING NOVA CITY OPERATING-DAY SIMULATION\n"
        << "============================================================\n";

    for (const DemandPeriod& period : demandPeriods) {
        simulatePeriod(
            period,
            samplePrintLimitPerPeriod,
            serviceStates);
    }

    for (const auto& [routeID, state] :
         serviceStates) {

        (void)routeID;

        allServiceStats.push_back(
            state);
    }

    std::sort(
        allServiceStats.begin(),
        allServiceStats.end(),
        [](
            const RouteServiceState& first,
            const RouteServiceState& second) {

            return first.routeID
                < second.routeID;
        });

    std::cout
        << "\nOperating-day simulation complete. "
        << "Total journey records: "
        << statisticsManager
               .getRecords()
               .size()
        << "\n";
}

StatisticsManager&
SimulationManager::getStatisticsManager() {
    return statisticsManager;
}

const StatisticsManager&
SimulationManager::getStatisticsManager() const {
    return statisticsManager;
}

const std::vector<RouteServiceState>&
SimulationManager::getServiceStats() const {
    return allServiceStats;
}

const std::vector<DemandPeriod>&
SimulationManager::getDemandPeriods() const {
    return demandPeriods;
}
