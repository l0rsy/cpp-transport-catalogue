#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <stdexcept>

#include "transport_catalogue.h"
#include "transport_router.h" 

namespace transport {

using namespace std;

size_t TransportCatalogue::PairStopHasher::operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stops) const { 
    return std::hash<const void*>{}(stops.first) * 37 +  
        std::hash<const void*>{}(stops.second);
} 

void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coords) {
    // Важно: используем deque для сохранения указателей при добавлении новых элементов
    stops_.push_back({name, coords});
    const domain::Stop* new_stop = &stops_.back();
    stop_name_to_stop_[new_stop->name] = new_stop;
    
    // Инициализируем пустой набор автобусов для новой остановки
    stop_to_buses_[new_stop];
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip) {
    domain::Bus bus;
    bus.name = name;
    bus.is_roundtrip = is_roundtrip;

    // Находим все остановки по именам
    for (const auto& stop_name : stop_names) {
        auto it = stop_name_to_stop_.find(stop_name);
        if (it != stop_name_to_stop_.end()) {
            bus.stops.push_back(it->second);
        }
    }
    
    // Добавляем автобус в хранилище
    buses_.push_back(move(bus));
    const domain::Bus* new_bus = &buses_.back();
    
    // Обновляем индексы
    bus_name_to_bus_[new_bus->name] = new_bus;
    
    // Добавляем автобус во все его остановки
    for (const domain::Stop* stop : new_bus->stops) {
        stop_to_buses_[stop].insert(new_bus->name);
    }
}

void TransportCatalogue::AddDistance(const std::string& from, const std::string& to, int distance) {
    const domain::Stop* stop_from = GetStop(from);
    const domain::Stop* stop_to = GetStop(to);
    if (stop_from && stop_to) {
        stops_distances_[{stop_from, stop_to}] = distance;
    }
}

const domain::Bus* TransportCatalogue::GetBus(string_view name) const {
    auto it = bus_name_to_bus_.find(name);
    return it != bus_name_to_bus_.end() ? it->second : nullptr;
}

const domain::Stop* TransportCatalogue::GetStop(string_view name) const {
    auto it = stop_name_to_stop_.find(name);
    return it != stop_name_to_stop_.end() ? it->second : nullptr;
}

int TransportCatalogue::GetDistance(const domain::Stop* from, const domain::Stop* to) const {
    // Сначала ищем прямое расстояние
    auto it = stops_distances_.find({from, to});
    if (it != stops_distances_.end()) {
        return it->second;
    }
    // Если не найдено, ищем обратное расстояние
    it = stops_distances_.find({to, from});
    if (it != stops_distances_.end()) {
        return it->second;
    }
    // Если расстояние не задано, вычисляем географическое
    double geo_dist = geo::ComputeDistance(from->coordinates, to->coordinates);
    int result = static_cast<int>(geo_dist);
    return result;
}

optional<domain::BusInfo> TransportCatalogue::GetBusInfo(string_view bus_name) const {
    const domain::Bus* bus = GetBus(bus_name);
    if (!bus || bus->stops.empty()) {
        return nullopt;
    }

    domain::BusInfo info;

    // Вычисляем количество остановок
    if (bus->is_roundtrip) {
        info.stops_count = bus->stops.size();
    } else {
        info.stops_count = bus->stops.size() * 2 - 1;
    }

    // Вычисляем количество уникальных остановок
    unordered_set<const domain::Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    info.unique_stops_count = unique_stops.size();

    // Вычисляем длины маршрутов
    double road_length = 0.0;
    double geo_length = 0.0;

    if (bus->is_roundtrip) {
        // Кольцевой маршрут
        for (size_t i = 1; i < bus->stops.size(); ++i) {
            const domain::Stop* from = bus->stops[i - 1];
            const domain::Stop* to = bus->stops[i];
            
            int dist = GetDistance(from, to);
            double geo_dist = geo::ComputeDistance(from->coordinates, to->coordinates);
            
            road_length += dist;
            geo_length += geo_dist;
        }
    } else {
        // Прямой путь
        for (size_t i = 1; i < bus->stops.size(); ++i) {
            const domain::Stop* from = bus->stops[i - 1];
            const domain::Stop* to = bus->stops[i];
            
            int dist = GetDistance(from, to);
            double geo_dist = geo::ComputeDistance(from->coordinates, to->coordinates);
            
            road_length += dist;
            geo_length += geo_dist;
            
        }

        for (size_t i = bus->stops.size() - 1; i > 0; --i) {
            const domain::Stop* from = bus->stops[i];
            const domain::Stop* to = bus->stops[i - 1];
            
            int dist = GetDistance(from, to);
            double geo_dist = geo::ComputeDistance(from->coordinates, to->coordinates);
            
            road_length += dist;
            geo_length += geo_dist;
            
        }
    }

    info.route_length = road_length;
    
    if (geo_length > 0) {
        info.curvature = road_length / geo_length;
    } else {
        info.curvature = 1.0;
    }

    return info;
}

optional<domain::StopInfo> TransportCatalogue::GetStopInfo(string_view stop_name) const {
    const domain::Stop* stop = GetStop(stop_name);
    if (!stop) {
        return nullopt;
    }

    domain::StopInfo info;
    
    auto it = stop_to_buses_.find(stop);
    if (it != stop_to_buses_.end()) {
        for (const auto& bus_name : it->second) {
            info.buses.insert(string(bus_name));
        }
    }
    
    return info;
}

vector<const domain::Bus*> TransportCatalogue::GetAllBusesSorted() const {
    vector<const domain::Bus*> result;
    for (const auto& [name, bus] : bus_name_to_bus_) {
        result.push_back(bus);
    }
    
    // Сортируем по имени
    sort(result.begin(), result.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
        return lhs->name < rhs->name;
    });
    
    return result;
}

vector<const domain::Stop*> TransportCatalogue::GetStopsUsedInRoutes() const {
    vector<const domain::Stop*> result;
    
    for (const auto& [stop, buses] : stop_to_buses_) {
        if (!buses.empty()) {
            result.push_back(stop);
        }
    }
    
    return result;
}

int TransportCatalogue::GetDistanceByRoad(const domain::Stop* from, const domain::Stop* to) const {
    auto it = stops_distances_.find({from, to});
    if (it != stops_distances_.end()) {
        return it->second;
    }
    return 0;
}

double TransportCatalogue::GetDistanceBetween(const domain::Stop* from, const domain::Stop* to) const {
    return static_cast<double>(GetDistance(from, to));
}

const unordered_map<string_view, const domain::Stop*>& TransportCatalogue::GetAllStops() const {
    return stop_name_to_stop_;
}

const unordered_map<string_view, const domain::Bus*>& TransportCatalogue::GetAllBuses() const {
    return bus_name_to_bus_;
}

int TransportCatalogue::GetStopsCount() const {
    return stops_.size();
}

void TransportCatalogue::SetRoutingSettings(const domain::RoutingSettings& settings) {
    routing_settings_ = settings;
    router_built_ = false;  
}

const domain::RoutingSettings& TransportCatalogue::GetRoutingSettings() const {
    return routing_settings_;
}

void TransportCatalogue::BuildRouter() {
    if (!router_built_) {
        router_ = make_shared<TransportRouter>(*this, routing_settings_);
        router_->BuildGraph();
        router_built_ = true;
    }
}

shared_ptr<TransportRouter> TransportCatalogue::GetRouter() const {
    if (!router_built_) {
        throw runtime_error("Router has not been built yet. Call BuildRouter() first.");
    }
    return router_;
}

} // namespace transport