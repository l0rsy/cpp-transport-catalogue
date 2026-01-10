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

struct RoutingSettings {
    int bus_wait_time = 0;     // в минутах
    double bus_velocity = 0.0; // в км/ч
};

struct RouteItem {
    std::string type;
    std::string stop_name;
    std::string bus; 
    int span_count = 0;
    double time = 0.0;
};

struct RouteResponse {
    int request_id = 0;
    double total_time = 0.0;
    std::vector<RouteItem> items;
    std::string error_message;
};

} // namespace domain