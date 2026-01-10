#pragma once

#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>

#include "graph.h"
#include "router.h"
#include "domain.h"

// Вместо #include "transport_catalogue.h" используем forward declaration
namespace transport {
    class TransportCatalogue;  // Forward declaration
}

namespace transport {

class TransportRouter {
public:
    struct EdgeInfo {
        std::string bus_name;
        int span_count = 0;
        std::string from_stop;
        std::string to_stop;
    };
    
    TransportRouter(const TransportCatalogue& catalogue, const domain::RoutingSettings& settings);
    
    void BuildGraph();
    std::optional<domain::RouteResponse> FindRoute(std::string_view from, std::string_view to) const;
    
private:
    struct VertexInfo {
        std::string stop_name;
        bool is_wait; // true - wait vertex, false - bus vertex
    };
    
    void AddBusEdges(const domain::Bus* bus);
    double ComputeTravelTimeBetween(const domain::Stop* from, const domain::Stop* to,
                                   size_t from_idx, size_t to_idx, 
                                   const std::vector<const domain::Stop*>& stops) const;
    
    const TransportCatalogue& catalogue_;
    domain::RoutingSettings settings_;
    
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    
    std::unordered_map<std::string, graph::VertexId> wait_vertices_;
    std::unordered_map<std::string, graph::VertexId> bus_vertices_;
    std::unordered_map<graph::EdgeId, EdgeInfo> edges_info_;
    std::vector<VertexInfo> vertices_info_;
};

} // namespace transport