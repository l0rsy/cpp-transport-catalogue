#include <cmath>
#include <stdexcept>
#include <string>

#include "transport_router.h"
#include "transport_catalogue.h" 

namespace transport {

using namespace std;

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, 
                                 const domain::RoutingSettings& settings)
    : catalogue_(catalogue)
    , settings_(settings) {
}

void TransportRouter::BuildGraph() {
    // Очищаем предыдущие данные
    wait_vertices_.clear();
    bus_vertices_.clear();
    edges_info_.clear();
    vertices_info_.clear();
    
    // Подсчитать количество вершин: 2 * количество остановок
    size_t stop_count = catalogue_.GetAllStops().size();
    size_t vertex_count = stop_count * 2;
    
    graph_ = make_unique<graph::DirectedWeightedGraph<double>>(vertex_count);
    vertices_info_.resize(vertex_count);
    
    // Создать вершины для каждой остановки
    size_t vertex_id = 0;
    for (const auto& [name, stop] : catalogue_.GetAllStops()) {
        // Wait вершина
        wait_vertices_[stop->name] = vertex_id;
        vertices_info_[vertex_id] = {stop->name, true};
        vertex_id++;
        
        // Bus вершина
        bus_vertices_[stop->name] = vertex_id;
        vertices_info_[vertex_id] = {stop->name, false};
        vertex_id++;
    }
    
    // Добавить Wait ребра
    for (const auto& [stop_name, wait_vertex] : wait_vertices_) {
        graph::Edge<double> edge;
        edge.from = wait_vertex;
        edge.to = bus_vertices_.at(stop_name);
        edge.weight = settings_.bus_wait_time;
        
        graph::EdgeId id = graph_->AddEdge(edge);
        edges_info_[id] = {"", 0, stop_name, stop_name}; // Wait edge
    }
    
    // Добавить Bus ребра для каждого маршрута
    for (const auto& [name, bus] : catalogue_.GetAllBuses()) {
        AddBusEdges(bus);
    }
    
    // Создать роутер
    router_ = make_unique<graph::Router<double>>(*graph_);
}

void TransportRouter::AddBusEdges(const domain::Bus* bus) {
    const auto& stops = bus->stops;
    
    // Для каждого возможного отрезка на маршруте
    for (size_t i = 0; i < stops.size(); ++i) {
        for (size_t j = i + 1; j < stops.size(); ++j) {
            double travel_time = ComputeTravelTimeBetween(stops[i], stops[j], i, j, stops);
            
            // Прямое направление
            graph::Edge<double> edge;
            edge.from = bus_vertices_.at(stops[i]->name);
            edge.to = wait_vertices_.at(stops[j]->name);
            edge.weight = travel_time;
            
            graph::EdgeId id = graph_->AddEdge(edge);
            edges_info_[id] = {bus->name, static_cast<int>(j - i), 
                              stops[i]->name, stops[j]->name};
        }
    }
    
    // Если маршрут не кольцевой, добавляем обратные ребра
    if (!bus->is_roundtrip) {
        for (size_t i = stops.size() - 1; i > 0; --i) {
            for (size_t j = i - 1; j < i; --j) {
                double travel_time = ComputeTravelTimeBetween(stops[i], stops[j], i, j, stops);
                
                graph::Edge<double> edge;
                edge.from = bus_vertices_.at(stops[i]->name);
                edge.to = wait_vertices_.at(stops[j]->name);
                edge.weight = travel_time;
                
                graph::EdgeId id = graph_->AddEdge(edge);
                edges_info_[id] = {bus->name, static_cast<int>(i - j), 
                                  stops[i]->name, stops[j]->name};
            }
        }
    }
}

double TransportRouter::ComputeTravelTimeBetween(const domain::Stop* from [[maybe_unused]], 
                                                const domain::Stop* to [[maybe_unused]],
                                                size_t from_idx, 
                                                size_t to_idx,
                                                const vector<const domain::Stop*>& stops) const {
    double distance = 0.0;
    
    // Суммируем расстояния между последовательными остановками
    if (from_idx < to_idx) {
        for (size_t i = from_idx; i < to_idx; ++i) {
            distance += catalogue_.GetDistanceBetween(stops[i], stops[i + 1]);
        }
    } else {
        // Для обратного направления
        for (size_t i = from_idx; i > to_idx; --i) {
            distance += catalogue_.GetDistanceBetween(stops[i], stops[i - 1]);
        }
    }
    
    // Время в минутах = (расстояние в метрах) / (скорость в м/мин)
    // скорость в м/мин = (velocity км/ч) * (1000 м / 60 мин)
    double speed_m_per_min = settings_.bus_velocity * 1000.0 / 60.0;
    return distance / speed_m_per_min;
}

optional<domain::RouteResponse> TransportRouter::FindRoute(
    string_view from, string_view to) const {
    
    if (!wait_vertices_.count(string(from)) || 
        !wait_vertices_.count(string(to))) {
        return nullopt;
    }
    
    graph::VertexId start = wait_vertices_.at(string(from));
    graph::VertexId finish = wait_vertices_.at(string(to));
    
    auto route = router_->BuildRoute(start, finish);
    
    if (!route) {
        return nullopt;
    }
    
    domain::RouteResponse response;
    response.total_time = route->weight;
    
    for (graph::EdgeId edge_id : route->edges) {
        const auto& edge_info = edges_info_.at(edge_id);
        
        if (edge_info.bus_name.empty()) {
            // Wait edge
            domain::RouteItem item;
            item.type = "Wait";
            item.stop_name = edge_info.from_stop;
            item.time = settings_.bus_wait_time;
            response.items.push_back(item);
        } else {
            // Bus edge
            domain::RouteItem item;
            item.type = "Bus";
            item.bus = edge_info.bus_name;
            item.span_count = edge_info.span_count;
            item.time = graph_->GetEdge(edge_id).weight;
            response.items.push_back(item);
        }
    }
    
    return response;
}

} // namespace transport