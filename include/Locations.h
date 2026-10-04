#ifndef LOCATIONS_H
#define LOCATIONS_H

#include <array>
#include <string>

constexpr int NUM_LOCATIONS = 10;

enum Location {
    NORTH_RESIDENTIAL = 0,
    UNIVERSITY,
    CENTRAL_STATION,
    GENERAL_HOSPITAL,
    SHOPPING_MALL,
    BUSINESS_DISTRICT,
    SOUTH_RESIDENTIAL,
    INDUSTRIAL_ZONE,
    AIRPORT,
    SPORTS_COMPLEX
};

inline const std::array<std::string, NUM_LOCATIONS> LOCATION_NAMES = {
    "North Residential Area",
    "University",
    "Central Station",
    "General Hospital",
    "Shopping Mall",
    "Business District",
    "South Residential Area",
    "Industrial Zone",
    "Airport",
    "Sports Complex"
};

inline const std::string& locationName(int location) {
    static const std::string unknown = "Unknown Location";
    if (location < 0 || location >= NUM_LOCATIONS) {
        return unknown;
    }
    return LOCATION_NAMES[location];
}

#endif
