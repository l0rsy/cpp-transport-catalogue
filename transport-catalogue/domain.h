#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip = false;
};

struct StopInfo {
    std::set<std::string> buses;
};

struct BusInfo {
    size_t stops_count;
    size_t unique_stops_count;
    double route_length;
    double curvature;
};

} // namespace domain