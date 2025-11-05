#pragma once
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <set>

#include "geo.h"

// Пространство имен для изоляции транспортного справочника
namespace transport {

struct Stop {
    std::string name; 
    Coordinates coordinates; 
};

struct Bus {
    std::string name; 
    std::vector<const Stop*> stops; 
    bool is_roundtrip; // Флаг кольцевого маршрута
};

class TransportCatalogue {
public:

    void AddStop(const std::string& name, Coordinates coords); 
    void AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip); 
    void AddDistance(const std::string& from, const std::string& to, int distance);

    const Bus* GetBus(std::string_view name) const;
    const Stop* GetStop(std::string_view name) const; 
    int GetDistance(const Stop* from, const Stop* to) const;

    struct BusInfo {
        size_t stops_count;
        size_t unique_stops_count; 
        double route_length; 
        double curvature;
    };

    struct StopInfo {
        std::set<std::string_view> buses; 
    };

    std::optional<BusInfo> GetBusInfo(std::string_view bus_name) const; 
    std::optional<StopInfo> GetStopInfo(std::string_view stop_name) const; 

private:
    std::deque<Stop> stops_; 
    std::deque<Bus> buses_;

    // Словари для хранения данных про остановки и автобусы
    std::unordered_map<std::string_view, const Stop*> stop_name_to_stop_;
    std::unordered_map<std::string_view, const Bus*> bus_name_to_bus_;
    std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_; 


    // Хэшер для хранения расстояния между остановками
    struct PairStopHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const;
    };

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopHasher> stops_distances_;

    
    
};

} // namespace transport

using transport::TransportCatalogue; // Делаем данный класс доступным в глобальном namespace