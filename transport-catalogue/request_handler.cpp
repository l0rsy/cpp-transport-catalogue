// request_handler.cpp
#include "request_handler.h"

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

} // namespace transport