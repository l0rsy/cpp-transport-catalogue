#pragma once

#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <vector>
#include "transport_catalogue.h"

namespace map_renderer {

inline const double EPSILON = 1e-6;

inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) 
    {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        } else {
            zoom_coeff_ = 0;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    double width = 1200.0;
    double height = 1200.0;
    double padding = 50.0;
    double line_width = 14.0;
    double stop_radius = 5.0;
    int bus_label_font_size = 20;
    svg::Point bus_label_offset = {7.0, 15.0};
    int stop_label_font_size = 20;
    svg::Point stop_label_offset = {7.0, -3.0};
    std::string underlayer_color = "white";
    double underlayer_width = 3.0;
    std::vector<std::string> color_palette;
    std::string font_family = "Verdana";
};

class MapRenderer {
public:
    void SetSettings(const RenderSettings& settings) {
        settings_ = settings;
    }
    
    svg::Document RenderMap(const transport::TransportCatalogue& catalogue) const;

private:
    RenderSettings settings_;
    
    void RenderBusLines(svg::Document& doc, 
                       const std::vector<const domain::Bus*>& buses,
                       const SphereProjector& projector) const;
    
    void RenderBusLabels(svg::Document& doc,
                        const std::vector<const domain::Bus*>& buses,
                        const SphereProjector& projector) const;
    
    void RenderStopPoints(svg::Document& doc,
                         const std::vector<const domain::Stop*>& stops,
                         const SphereProjector& projector) const;
    
    void RenderStopLabels(svg::Document& doc,
                         const std::vector<const domain::Stop*>& stops,
                         const SphereProjector& projector) const;
};

} // namespace map_renderer