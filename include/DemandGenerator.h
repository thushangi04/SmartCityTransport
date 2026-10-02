#ifndef DEMAND_GENERATOR_H
#define DEMAND_GENERATOR_H

#include <random>
#include <string>
#include <utility>
#include <vector>

#include "Passenger.h"
#include "SimulationTypes.h"

class DemandGenerator {
private:
    int nextPassengerID;
    unsigned int initialSeed;
    std::mt19937 rng;

    std::string createPassengerID();
    std::string createDepartureTime(int startHour, int endHour);
    int chooseFrom(const std::vector<int>& values);
    int weightedChoose(const std::vector<int>& values,
                       const std::vector<int>& weights);
    std::pair<int, int> chooseTrip(const DemandPeriod& period);

public:
    explicit DemandGenerator(unsigned int seed = 42);

    void reset();

    std::vector<Passenger> generatePassengers(
        const DemandPeriod& period);
};

#endif
