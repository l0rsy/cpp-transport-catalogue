#include "json_reader.h"
#include "json_builder.h"
#include <algorithm>
#include <string>
#include <iomanip>
#include <sstream>

namespace json_reader {

using namespace std::literals;

JsonReader::JsonReader(const std::string& json_str) 
    : input_doc_([&json_str]() {
        std::istringstream input(json_str);
        return json::Load(input);
    }()) {
}

JsonReader::JsonReader(const json::Document& doc)
    : input_doc_(doc) {
}

void JsonReader::LoadData() {
    const auto& root_map = input_doc_.GetRoot().AsMap();
    
    if (root_map.count("base_requests"s)) {
        ParseBaseRequests(root_map.at("base_requests"s).AsArray());
    }
}

std::string ColorToString(const json::Node& color_node) {
    if (color_node.IsString()) {
        return color_node.AsString();
    } else if (color_node.IsArray()) {
        const auto& color_array = color_node.AsArray();
        if (color_array.size() == 3) {
            // RGB формат: [255, 160, 0]
            int r = color_array[0].AsInt();
            int g = color_array[1].AsInt();
            int b = color_array[2].AsInt();
            
            std::ostringstream out;
            out << "rgb(" << r << "," << g << "," << b << ")";
            return out.str();
            
        } else if (color_array.size() == 4) {
            // RGBA формат: [255, 255, 255, 0.85]
            int r = color_array[0].AsInt();
            int g = color_array[1].AsInt();
            int b = color_array[2].AsInt();
            double a = color_array[3].AsDouble();
            
            std::ostringstream out;
            out << "rgba(" << r << "," << g << "," << b << "," << a << ")";
            return out.str();
        }
    }
    return "black"; // fallback
}

map_renderer::RenderSettings JsonReader::GetRenderSettings() const {
    const auto& root_map = input_doc_.GetRoot().AsMap();
    map_renderer::RenderSettings settings;
    
    if (root_map.count("render_settings"s)) {
        const auto& render_settings = root_map.at("render_settings"s).AsMap();
        
        settings.width = render_settings.at("width"s).AsDouble();
        settings.height = render_settings.at("height"s).AsDouble();
        settings.padding = render_settings.at("padding"s).AsDouble();
        settings.line_width = render_settings.at("line_width"s).AsDouble();
        settings.stop_radius = render_settings.at("stop_radius"s).AsDouble();
        settings.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
        
        const auto& bus_offset = render_settings.at("bus_label_offset"s).AsArray();
        settings.bus_label_offset = {bus_offset[0].AsDouble(), bus_offset[1].AsDouble()};
        
        settings.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
        
        const auto& stop_offset = render_settings.at("stop_label_offset"s).AsArray();
        settings.stop_label_offset = {stop_offset[0].AsDouble(), stop_offset[1].AsDouble()};
        
        // Нормальный парсинг underlayer_color
        settings.underlayer_color = ColorToString(render_settings.at("underlayer_color"s));
        
        settings.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
        
        // Нормальный парсинг color_palette
        const auto& palette = render_settings.at("color_palette"s).AsArray();
        for (const auto& color_node : palette) {
            settings.color_palette.push_back(ColorToString(color_node));
        }
    }
    return settings;
}

json::Document JsonReader::ProcessRequests() {
    const auto& root_map = input_doc_.GetRoot().AsMap();
    
    if (root_map.count("stat_requests"s)) {
        json::Array responses = ProcessStatRequests(root_map.at("stat_requests"s).AsArray());
        return json::Document(responses);
    }
    
    return json::Document(json::Array{});
}

void JsonReader::ParseBaseRequests(const json::Array& requests) {
    // 1. Сначала добавляем ВСЕ остановки (без расстояний)
    for (const auto& request_node : requests) {
        const auto& request_map = request_node.AsMap();
        if (request_map.at("type"s).AsString() == "Stop"s) {
            std::string name = request_map.at("name"s).AsString();
            double lat = request_map.at("latitude"s).AsDouble();
            double lng = request_map.at("longitude"s).AsDouble();
            catalogue_.AddStop(name, {lat, lng});
        }
    }
    
    // 2. Теперь добавляем расстояния (все остановки уже существуют)
    for (const auto& request_node : requests) {
        const auto& request_map = request_node.AsMap();
        if (request_map.at("type"s).AsString() == "Stop"s) {
            std::string name = request_map.at("name"s).AsString();
            
            // Добавляем расстояния до других остановок
            if (request_map.count("road_distances"s)) {
                const auto& distances = request_map.at("road_distances"s).AsMap();
                for (const auto& [to_stop, distance_node] : distances) {
                    int distance = distance_node.AsInt();
                    catalogue_.AddDistance(name, to_stop, distance);
                }
            }
        }
    }
    
    // 3. Затем обрабатываем автобусы
    for (const auto& request_node : requests) {
        const auto& request_map = request_node.AsMap();
        if (request_map.at("type"s).AsString() == "Bus"s) {
            ParseBus(request_map);
        }
    }
}

// Новый метод только для парсинга расстояний
void JsonReader::ParseStopDistances(const json::Dict& stop_dict) {
    std::string name = stop_dict.at("name"s).AsString();
    
    // Добавляем расстояния до других остановок
    if (stop_dict.count("road_distances"s)) {
        const auto& distances = stop_dict.at("road_distances"s).AsMap();
        for (const auto& [to_stop, distance_node] : distances) {
            int distance = distance_node.AsInt();
            catalogue_.AddDistance(name, to_stop, distance);
        }
    }
}

void JsonReader::ParseBus(const json::Dict& bus_dict) {
    std::string name = bus_dict.at("name"s).AsString();
    bool is_roundtrip = bus_dict.at("is_roundtrip"s).AsBool();
    
    std::vector<std::string> stop_names;
    for (const auto& stop_node : bus_dict.at("stops"s).AsArray()) {
        stop_names.push_back(stop_node.AsString());
    }
    
    catalogue_.AddBus(name, stop_names, is_roundtrip);
}

json::Array JsonReader::ProcessStatRequests(const json::Array& requests) {
    json::Array responses;
    
    for (const auto& request_node : requests) {
        const auto& request_map = request_node.AsMap();
        std::string type = request_map.at("type"s).AsString();
        
        json::Node response;
        if (type == "Bus"s) {
            response = ProcessBusRequest(request_map);
        } else if (type == "Stop"s) {
            response = ProcessStopRequest(request_map);
        } else if (type == "Map"s) { 
            response = ProcessMapRequest(request_map);
        }
        
        responses.push_back(response);
    }
    
    return responses;
}

json::Node JsonReader::ProcessBusRequest(const json::Dict& request) {
    std::string bus_name = request.at("name"s).AsString();
    int id = request.at("id"s).AsInt();
    
    transport::RequestHandler request_handler(catalogue_);
    auto bus_info = request_handler.GetBusInfo(bus_name);
    
    if (!bus_info) {
        // Используем Builder для ошибки
        return json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(id)
                .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
    }
    
    // Используем Builder для успешного ответа
    return json::Builder{}
        .StartDict()
            .Key("curvature"s).Value(bus_info->curvature)
            .Key("request_id"s).Value(id)
            .Key("route_length"s).Value(static_cast<int>(bus_info->route_length))
            .Key("stop_count"s).Value(static_cast<int>(bus_info->stops_count))
            .Key("unique_stop_count"s).Value(static_cast<int>(bus_info->unique_stops_count))
        .EndDict()
        .Build();
}

json::Node JsonReader::ProcessStopRequest(const json::Dict& request) {
    std::string stop_name = request.at("name"s).AsString();
    int id = request.at("id"s).AsInt();
    
    transport::RequestHandler request_handler(catalogue_);
    auto stop_info = request_handler.GetStopInfo(stop_name);
    
    if (!stop_info) {
        // Используем Builder для ошибки
        return json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(id)
                .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
    }
    
    std::vector<std::string> buses_sorted;
    for (const auto& bus_name : stop_info->buses) {
        buses_sorted.push_back(std::string(bus_name));
    }
    std::sort(buses_sorted.begin(), buses_sorted.end());
    
    json::Array buses_array;
    for (const auto& bus_name : buses_sorted) {
        buses_array.push_back(bus_name);
    }
    
    // Используем Builder для успешного ответа
    return json::Builder{}
        .StartDict()
            .Key("buses"s).Value(std::move(buses_array))  // Перемещаем массив
            .Key("request_id"s).Value(id)
        .EndDict()
        .Build();
}

json::Node JsonReader::ProcessMapRequest(const json::Dict& request) {
    int id = request.at("id"s).AsInt();
    
    transport::RequestHandler request_handler(catalogue_);
    auto render_settings = GetRenderSettings();
    svg::Document map_document = request_handler.RenderMap(render_settings);
    
    std::ostringstream svg_stream;
    map_document.Render(svg_stream);
    std::string svg_string = svg_stream.str();
    
    // Используем Builder для ответа
    return json::Builder{}
        .StartDict()
            .Key("map"s).Value(svg_string)
            .Key("request_id"s).Value(id)
        .EndDict()
        .Build();
}

} // namespace json_reader