#ifndef SIMULATION_MANAGER_H
#define SIMULATION_MANAGER_H

#include <unordered_map>
#include <vector>

#include "Bus.h"
#include "DemandGenerator.h"
#include "Graph.h"
#include "JourneySimulator.h"
#include "SimulationTypes.h"
#include "StatisticsManager.h"
#include "Train.h"

class SimulationManager {
private:
    Graph& graph;
    std::vector<Bus> buses;
    std::vector<Train> trains;
    DemandGenerator demandGenerator;
    JourneySimulator journeySimulator;
    StatisticsManager statisticsManager;
    std::vector<DemandPeriod> demandPeriods;
    std::vector<RouteServiceState> allServiceStats;

    std::unordered_map<std::string, RouteServiceState>
    createServiceStates() const;

    JourneyRecord makeRecord(
        const Passenger& passenger,
        const Journey& journey,
        const DemandPeriod& period) const;

    void simulatePeriod(
        const DemandPeriod& period,
        int samplePrintLimit,
        std::unordered_map<std::string, RouteServiceState>& serviceStates);

public:
    explicit SimulationManager(
        Graph& graphRef,
        unsigned int seed = 42);

    void runOperatingDaySimulation(
        int samplePrintLimitPerPeriod = 2);

    StatisticsManager& getStatisticsManager();
    const StatisticsManager& getStatisticsManager() const;
    const std::vector<RouteServiceState>& getServiceStats() const;
    const std::vector<DemandPeriod>& getDemandPeriods() const;
};

#endif
