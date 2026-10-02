#include "DemandGenerator.h"
#include "Locations.h"

#include <iomanip>
#include <sstream>

DemandGenerator::DemandGenerator(unsigned int seed)
    : nextPassengerID(1),
      initialSeed(seed),
      rng(seed) {}

void DemandGenerator::reset() {
    nextPassengerID = 1;
    rng.seed(initialSeed);
}

std::string DemandGenerator::createPassengerID() {
    std::ostringstream out;
    out << 'P'
        << std::setw(3)
        << std::setfill('0')
        << nextPassengerID++;
    return out.str();
}

std::string DemandGenerator::createDepartureTime(
    int startHour,
    int endHour) {

    const int startMinute = startHour * 60;
    const int endMinute = endHour * 60 - 1;

    std::uniform_int_distribution<int> dist(
        startMinute,
        endMinute);

    const int absoluteMinute = dist(rng);
    const int hour = absoluteMinute / 60;
    const int minute = absoluteMinute % 60;

    std::ostringstream out;
    out << std::setw(2)
        << std::setfill('0')
        << hour
        << ':'
        << std::setw(2)
        << std::setfill('0')
        << minute;

    return out.str();
}

int DemandGenerator::chooseFrom(
    const std::vector<int>& values) {

    std::uniform_int_distribution<std::size_t> dist(
        0,
        values.size() - 1);

    return values[dist(rng)];
}

int DemandGenerator::weightedChoose(
    const std::vector<int>& values,
    const std::vector<int>& weights) {

    std::discrete_distribution<int> dist(
        weights.begin(),
        weights.end());

    return values[dist(rng)];
}

std::pair<int, int> DemandGenerator::chooseTrip(
    const DemandPeriod& period) {

    int source = -1;
    int destination = -1;

    if (period.startHour == 7) {
        source = weightedChoose(
            {NORTH_RESIDENTIAL,
             SOUTH_RESIDENTIAL,
             GENERAL_HOSPITAL},
            {45, 45, 10});

        destination = weightedChoose(
            {UNIVERSITY,
             BUSINESS_DISTRICT,
             INDUSTRIAL_ZONE,
             CENTRAL_STATION,
             AIRPORT},
            {35, 30, 18, 10, 7});
    }
    else if (period.startHour == 16) {
        source = weightedChoose(
            {UNIVERSITY,
             BUSINESS_DISTRICT,
             INDUSTRIAL_ZONE,
             CENTRAL_STATION},
            {30, 35, 25, 10});

        destination = weightedChoose(
            {NORTH_RESIDENTIAL,
             SOUTH_RESIDENTIAL,
             SHOPPING_MALL,
             SPORTS_COMPLEX},
            {40, 40, 12, 8});
    }
    else if (period.startHour == 12) {
        source = chooseFrom({
            NORTH_RESIDENTIAL,
            SOUTH_RESIDENTIAL,
            UNIVERSITY,
            BUSINESS_DISTRICT,
            CENTRAL_STATION
        });

        destination = weightedChoose(
            {SHOPPING_MALL,
             GENERAL_HOSPITAL,
             CENTRAL_STATION,
             SPORTS_COMPLEX,
             UNIVERSITY},
            {35, 20, 20, 10, 15});
    }
    else {
        std::uniform_int_distribution<int> locationDist(
            0,
            NUM_LOCATIONS - 1);

        source = locationDist(rng);
        destination = locationDist(rng);
    }

    while (destination == source) {
        std::uniform_int_distribution<int> locationDist(
            0,
            NUM_LOCATIONS - 1);

        destination = locationDist(rng);
    }

    return {source, destination};
}

std::vector<Passenger> DemandGenerator::generatePassengers(
    const DemandPeriod& period) {

    std::vector<Passenger> passengers;
    passengers.reserve(period.passengerCount);

    for (int i = 0;
         i < period.passengerCount;
         ++i) {

        auto [source, destination] = chooseTrip(period);

        passengers.emplace_back(
            createPassengerID(),
            source,
            destination,
            createDepartureTime(
                period.startHour,
                period.endHour));
    }

    return passengers;
}
