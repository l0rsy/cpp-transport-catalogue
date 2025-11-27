#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

#include <iostream>

namespace json_reader {

class JsonReader {
public:
    JsonReader(transport::TransportCatalogue& catalogue, std::istream& input = std::cin);
    
    void LoadData();
    void ProcessRequests(std::ostream& output = std::cout);

    map_renderer::RenderSettings GetRenderSettings() const;

private:
    void ParseBaseRequests(const json::Array& requests);
    void ParseStopDistances(const json::Dict& stop_dict);
    void ParseBus(const json::Dict& bus_dict);
    
    json::Array ProcessStatRequests(const json::Array& requests);
    json::Node ProcessBusRequest(const json::Dict& request);
    json::Node ProcessStopRequest(const json::Dict& request);
    json::Node ProcessMapRequest(const json::Dict& request);

    transport::TransportCatalogue& catalogue_;
    transport::RequestHandler request_handler_;
    json::Document input_doc_;
};

} // namespace json_reader