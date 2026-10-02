#ifndef TRANSPORT_CONFIG_H
#define TRANSPORT_CONFIG_H

#include <string>

namespace TransportConfig {

constexpr int SERVICE_START_MINUTE = 5 * 60;
constexpr int SERVICE_END_MINUTE = 26 * 60;

constexpr double BUS_HEADWAY_MINUTES = 15.0;
constexpr double TRAIN_HEADWAY_MINUTES = 20.0;

constexpr double TRANSFER_PENALTY_MINUTES = 5.0;

inline double headwayForTransport(const std::string& transportType) {
    if (transportType == "Train") {
        return TRAIN_HEADWAY_MINUTES;
    }

    return BUS_HEADWAY_MINUTES;
}

}

#endif
