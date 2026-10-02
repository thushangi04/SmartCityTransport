#ifndef SIMULATION_TYPES_H
#define SIMULATION_TYPES_H

#include <string>
#include <vector>

enum class DemandLevel {
    LOW,
    MEDIUM,
    HIGH,
    VERY_HIGH
};

struct DemandPeriod {
    std::string periodName;
    int startHour = 0;
    int endHour = 0;
    DemandLevel level = DemandLevel::LOW;
    int passengerCount = 0;
};

struct RouteServiceState {
    std::string periodName;
    std::string routeID;
    std::string transportType;
    int capacityPerDeparture = 0;
    int departures = 0;
    double headwayMinutes = 0.0;
    int serviceStartMinute = 0;
    int serviceEndMinute = 0;
    int assignedBoardings = 0;
    int rejectedBoardings = 0;
    int maxQueueLength = 0;
    std::vector<int> boardedPerDeparture;
    std::vector<int> queuedForDeparture;
};

struct PerformanceMetrics {
    int generatedPassengers = 0;
    int completedJourneys = 0;
    int failedJourneys = 0;
    double successRate = 0.0;
    double averageJourneyTime = 0.0;
    double averageWaitingTime = 0.0;
    double averageInterchangeTime = 0.0;
    double averageDistance = 0.0;
    double averageTransfers = 0.0;
    double averageBusUtilization = 0.0;
    double averageTrainUtilization = 0.0;
    int maximumQueueLength = 0;
    int passengersLeftWaiting = 0;
};

#endif
