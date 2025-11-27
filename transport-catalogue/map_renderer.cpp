#include "map_renderer.h"

#include <iostream>

namespace map_renderer {

using namespace std;

svg::Document MapRenderer::RenderMap(const transport::TransportCatalogue& catalogue) const {
    svg::Document doc;
    
    auto buses = catalogue.GetAllBusesSorted();
    auto stops = catalogue.GetStopsUsedInRoutes();
    
    vector<geo::Coordinates> geo_coords;
    for (const auto& stop : stops) {
        geo_coords.push_back(stop->coordinates);
    }
    
    SphereProjector projector(geo_coords.begin(), geo_coords.end(), 
                             settings_.width, settings_.height, settings_.padding);
    
    RenderBusLines(doc, buses, projector);
    RenderBusLabels(doc, buses, projector);
    RenderStopPoints(doc, stops, projector);
    RenderStopLabels(doc, stops, projector);
    
    return doc;
}

void MapRenderer::RenderBusLines(svg::Document& doc, 
                                const std::vector<const domain::Bus*>& buses,
                                const SphereProjector& projector) const {
    size_t color_index = 0;
    
    for (const auto& bus : buses) {
        if (bus->stops.size() < 2) {
            continue;
        }
        
        const std::string& color = settings_.color_palette[color_index % settings_.color_palette.size()];
        svg::Polyline polyline;
        
        if (bus->is_roundtrip) {
            // Кольцевой маршрут - рисуем все остановки
            for (const auto& stop : bus->stops) {
                polyline.AddPoint(projector(stop->coordinates));
            }
        } else {
            // Некольцевой маршрут - рисуем прямой + обратный путь
            // Прямой путь
            for (const auto& stop : bus->stops) {
                polyline.AddPoint(projector(stop->coordinates));
            }
            // Обратный путь (кроме последней остановки, чтобы не дублировать)
            for (auto it = bus->stops.rbegin() + 1; it != bus->stops.rend(); ++it) {
                polyline.AddPoint(projector((*it)->coordinates));
            }
        }
        
        polyline.SetFillColor("none")
                .SetStrokeColor(color)
                .SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        doc.Add(polyline);
        color_index++;
    }
}

void MapRenderer::RenderBusLabels(svg::Document& doc,
                                 const std::vector<const domain::Bus*>& buses,
                                 const SphereProjector& projector) const {
    size_t color_index = 0;
    
    for (const auto& bus : buses) {
        if (bus->stops.empty()) {
            continue;
        }
        
        const std::string& color = settings_.color_palette[color_index % settings_.color_palette.size()];
        
        vector<const domain::Stop*> terminal_stops;
        
        if (bus->is_roundtrip) {
            terminal_stops.push_back(bus->stops.front());
        } else {
            terminal_stops.push_back(bus->stops.front());
            if (bus->stops.front() != bus->stops.back()) {
                terminal_stops.push_back(bus->stops.back());
            }
        }
        
        for (const auto& stop : terminal_stops) {
            svg::Point point = projector(stop->coordinates);
            
            svg::Text underlayer;
            underlayer.SetPosition(point)
                     .SetOffset(settings_.bus_label_offset)
                     .SetFontSize(settings_.bus_label_font_size)
                     .SetFontFamily(settings_.font_family)
                     .SetFontWeight("bold")
                     .SetData(bus->name)
                     .SetFillColor(settings_.underlayer_color)
                     .SetStrokeColor(settings_.underlayer_color)
                     .SetStrokeWidth(settings_.underlayer_width)
                     .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                     .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            
            svg::Text text;
            text.SetPosition(point)
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily(settings_.font_family)
                .SetFontWeight("bold")
                .SetData(bus->name)
                .SetFillColor(color);
            
            doc.Add(underlayer);
            doc.Add(text);
        }
        
        color_index++;
    }
}

void MapRenderer::RenderStopPoints(svg::Document& doc,
                                  const std::vector<const domain::Stop*>& stops,
                                  const SphereProjector& projector) const {
    // Создаем копию и сортируем остановки по имени
    std::vector<const domain::Stop*> sorted_stops = stops;
    std::sort(sorted_stops.begin(), sorted_stops.end(), 
              [](const domain::Stop* lhs, const domain::Stop* rhs) {
                  return lhs->name < rhs->name;
              });
    
    for (const auto& stop : sorted_stops) {
        svg::Point point = projector(stop->coordinates);
        
        svg::Circle circle;
        circle.SetCenter(point)
              .SetRadius(settings_.stop_radius)
              .SetFillColor("white");
        
        doc.Add(circle);
    }
}

void MapRenderer::RenderStopLabels(svg::Document& doc,
                                  const std::vector<const domain::Stop*>& stops,
                                  const SphereProjector& projector) const {
    // Создаем копию и сортируем остановки по имени
    std::vector<const domain::Stop*> sorted_stops = stops;
    std::sort(sorted_stops.begin(), sorted_stops.end(), 
              [](const domain::Stop* lhs, const domain::Stop* rhs) {
                  return lhs->name < rhs->name;
              });
    
    for (const auto& stop : sorted_stops) {
        svg::Point point = projector(stop->coordinates);
        
        // Подложка
        svg::Text underlayer;
        underlayer.SetPosition(point)
                 .SetOffset(settings_.stop_label_offset)
                 .SetFontSize(settings_.stop_label_font_size)
                 .SetFontFamily(settings_.font_family)
                 .SetData(stop->name)
                 .SetFillColor(settings_.underlayer_color)
                 .SetStrokeColor(settings_.underlayer_color)
                 .SetStrokeWidth(settings_.underlayer_width)
                 .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                 .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        // Основной текст
        svg::Text text;
        text.SetPosition(point)
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily(settings_.font_family)
            .SetData(stop->name)
            .SetFillColor("black");
        
        doc.Add(underlayer);
        doc.Add(text);
    }
}

} // namespace map_renderer