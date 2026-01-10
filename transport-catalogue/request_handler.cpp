#include "request_handler.h"
#include "transport_router.h"

namespace transport {

std::optional<domain::BusInfo> RequestHandler::GetBusInfo(std::string_view bus_name) const {
    return db_.GetBusInfo(bus_name);
}

std::optional<domain::StopInfo> RequestHandler::GetStopInfo(std::string_view stop_name) const {
    return db_.GetStopInfo(stop_name);
}

svg::Document RequestHandler::RenderMap(const map_renderer::RenderSettings& settings) const {
    map_renderer::MapRenderer renderer;
    renderer.SetSettings(settings);
    return renderer.RenderMap(db_);
}

std::optional<domain::RouteResponse> RequestHandler::GetRoute(
    std::string_view from, std::string_view to) const {
    
    auto router = db_.GetRouter();
    if (!router) {
        return std::nullopt;
    }
    
    return router->FindRoute(from, to);
}

} // namespace transport