#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

#include <string>

namespace json_reader {

class JsonReader {
public:
    // Конструктор из JSON строки
    explicit JsonReader(const std::string& json_str);
    
    // Конструктор из JSON документа
    explicit JsonReader(const json::Document& doc);
    
    // Загрузка данных в каталог
    void LoadData();
    
    // Обработка запросов - возвращает JSON документ с ответами
    json::Document ProcessRequests();
    
    // Получение настроек рендеринга
    map_renderer::RenderSettings GetRenderSettings() const;
    
    // Получение каталога
    transport::TransportCatalogue& GetCatalogue() { return catalogue_; }
    const transport::TransportCatalogue& GetCatalogue() const { return catalogue_; }

private:
    void ParseBaseRequests(const json::Array& requests);
    void ParseStopDistances(const json::Dict& stop_dict);
    void ParseBus(const json::Dict& bus_dict);
    
    json::Array ProcessStatRequests(const json::Array& requests);
    json::Node ProcessBusRequest(const json::Dict& request);
    json::Node ProcessStopRequest(const json::Dict& request);
    json::Node ProcessMapRequest(const json::Dict& request);

    transport::TransportCatalogue catalogue_;
    json::Document input_doc_;
};

} // namespace json_reader