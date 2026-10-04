#ifndef NETWORK_ANALYZER_H
#define NETWORK_ANALYZER_H

#include <map>
#include <string>
#include <vector>
#include "JourneyRecord.h"
#include "SimulationTypes.h"

struct AnalysisResult {
    std::map<std::string, int> routeUsage;
    std::map<int, int> stationUsage;
    std::map<int, int> transferUsage;
    std::map<int, int> sourceUsage;
    std::map<int, int> destinationUsage;

    std::string busiestBusRoute;
    std::string busiestTrainRoute;
    int busiestStation = -1;
    int leastUsedStation = -1;
    int busiestTransferStation = -1;
    std::string busiestPeriod;

    PerformanceMetrics metrics;
};

class NetworkAnalyzer {
public:
    AnalysisResult analyze(
        const std::vector<JourneyRecord>& records,
        const std::vector<RouteServiceState>& serviceStats,
        const std::vector<DemandPeriod>& demandPeriods) const;

    void printReport(
        const AnalysisResult& result,
        const std::vector<DemandPeriod>& demandPeriods) const;
};

#endif
