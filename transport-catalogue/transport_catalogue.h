#pragma once

#include "domain.h"

#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <set>

namespace transport {

class TransportCatalogue {
public:
    void AddStop(const std::string& name, geo::Coordinates coords);
    void AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip);
    void AddDistance(const std::string& from, const std::string& to, int distance);

    const domain::Bus* GetBus(std::string_view name) const;
    const domain::Stop* GetStop(std::string_view name) const;
    int GetDistance(const domain::Stop* from, const domain::Stop* to) const;

    std::optional<domain::BusInfo> GetBusInfo(std::string_view bus_name) const;
    std::optional<domain::StopInfo> GetStopInfo(std::string_view stop_name) const;

    std::vector<const domain::Bus*> GetAllBusesSorted() const;
    std::vector<const domain::Stop*> GetStopsUsedInRoutes() const;

private:
    std::deque<domain::Stop> stops_;
    std::deque<domain::Bus> buses_;

    std::unordered_map<std::string_view, const domain::Stop*> stop_name_to_stop_;
    std::unordered_map<std::string_view, const domain::Bus*> bus_name_to_bus_;
    std::unordered_map<const domain::Stop*, std::set<std::string>> stop_to_buses_;

    struct PairStopHasher {
        size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stops) const;
    };
    std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, PairStopHasher> stops_distances_;
};

} // namespace transport