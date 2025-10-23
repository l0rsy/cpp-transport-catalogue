#include "transport_catalogue.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace transport {

void TransportCatalogue::AddStop(std::string name, double lat, double lng) {
    AddStop(std::move(name), Coordinates{lat, lng});
}

void TransportCatalogue::AddStop(std::string name, Coordinates coords) {
    stops_.push_back({std::move(name), coords});
    const Stop* new_stop = &stops_.back();
    stop_name_to_stop_[new_stop->name] = new_stop;
    
    // Инициализируем пустой набор автобусов для новой остановки
    stop_to_buses_[new_stop];
}

void TransportCatalogue::AddBus(std::string name, const std::vector<std::string>& stop_names, bool is_roundtrip) {
    Bus bus;
    bus.name = std::move(name);
    bus.is_roundtrip = is_roundtrip;

    // Сначала находим все остановки
    for (const auto& stop_name : stop_names) {
        auto it = stop_name_to_stop_.find(stop_name);
        if (it != stop_name_to_stop_.end()) {
            bus.stops.push_back(it->second);
        }
    }
    
    // Добавляем автобус в основное хранилище
    buses_.push_back(std::move(bus));
    const Bus* new_bus = &buses_.back();
    
    // Теперь сохраняем индексы (string_view будут валидными)
    bus_name_to_bus_[new_bus->name] = new_bus;
    
    // Добавляем автобус во все его остановки
    for (const Stop* stop : new_bus->stops) {
        stop_to_buses_[stop].insert(new_bus->name);
    }
}

const Bus* TransportCatalogue::GetBus(std::string_view name) const {
    auto it = bus_name_to_bus_.find(name);
    if (it != bus_name_to_bus_.end()) {
        return it->second;
    }
    return nullptr;
}

const Stop* TransportCatalogue::GetStop(std::string_view name) const {
    auto it = stop_name_to_stop_.find(name);
    if (it != stop_name_to_stop_.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<TransportCatalogue::BusInfo> TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
    const Bus* bus = GetBus(bus_name);
    if (!bus || bus->stops.empty()) {
        return std::nullopt;
    }

    BusInfo info;

    if (bus->is_roundtrip) {
        info.stops_count = bus->stops.size();
    } else {
        info.stops_count = bus->stops.size() * 2 - 1;
    }

    std::unordered_set<const Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    info.unique_stops_count = unique_stops.size();

    info.route_length = 0.0;
    for (size_t i = 1; i < bus->stops.size(); i++) { 
        const Stop* from = bus->stops[i - 1];
        const Stop* to = bus->stops[i];
        info.route_length += ComputeDistance(
            from->coordinates,
            to->coordinates
        );
    }

    if (!bus->is_roundtrip) {
        for (size_t i = bus->stops.size() - 1; i > 0; --i) {
            const Stop* from = bus->stops[i];
            const Stop* to = bus->stops[i - 1];
            info.route_length += ComputeDistance(
                from->coordinates,
                to->coordinates
            );
        }
    } else {
        const Stop* first = bus->stops.front();
        const Stop* last = bus->stops.back();
        info.route_length += ComputeDistance(
            last->coordinates,
            first->coordinates
        );
    }

    return info;
}

std::optional<TransportCatalogue::StopInfo> TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    const Stop* stop = GetStop(stop_name);
    if (!stop) {
        return std::nullopt;
    }

    StopInfo info;
    
    auto it = stop_to_buses_.find(stop);
    if (it != stop_to_buses_.end()) {
        info.buses = it->second;
    }
    
    return info;
}

} // namespace transport