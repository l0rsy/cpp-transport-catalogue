// request_handler.h
#pragma once

#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"
#include "svg.h"

namespace transport {

class RequestHandler {
public:
    explicit RequestHandler(TransportCatalogue& db) : db_(db) {}

    // Методы для работы с транспортным каталогом
    std::optional<domain::BusInfo> GetBusInfo(std::string_view bus_name) const;
    std::optional<domain::StopInfo> GetStopInfo(std::string_view stop_name) const;

    svg::Document RenderMap() const;
    svg::Document RenderMap(const map_renderer::RenderSettings& settings) const;
    

private:
    TransportCatalogue& db_;
};

} // namespace transport