#pragma once
#include <deque>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <set>

#include "geo.h"


// Пространство имен для изоляции транспортного справочника
namespace transport {

struct Stop {
	std::string name; // Название остановки
	double latitude; // Широта
	double longitude; // Долгота
};

struct Bus {
	std::string name; // Название маршрута
	std::vector<const Stop*> stops; // Указатели на остановки маршрута
	bool is_roundtrip; // Флаг кольцевого маршрута
};

class TransportCatalogue {
public:
	void AddStop(std::string name, double lat, double lng); // Добавляет остановку в базу

	void AddBus(std::string name, const std::vector<std::string>& stop_names, bool is_roundtrip); // Добавляет маршрут в базу

	const Bus* GetBus(const std::string& name) const; // Поиск маршрута по имени

	const Stop* GetStop(const std::string& name) const; // Поиск остановки по имени

	struct BusInfo {
		size_t stops_count; // Кол-во остановок в маршруте
		size_t unique_stops_count; // Кол-во уникальных остановок в маршруте
		double route_length; // Длина маршрута в метрах
	};

	std::optional<BusInfo> GetBusInfo(const std::string& bus_name) const; // Получает статистику о маршруте

	struct StopInfo {
		std::set<std::string_view> buses; // Отсортированные названия автобусов
	};

	std::optional<StopInfo> GetStopInfo(const std::string& stop_name) const; // Получает статистику об остановке

private:
	std::deque<Stop> stops_; // Все остановки справочника
	std::deque<Bus> buses_; // Все автобусы справочника

	// Словари для хранения данных про остановки и автобусы
	std::unordered_map<std::string_view, const Stop*> stop_name_to_stop_;
	std::unordered_map<std::string_view, const Bus*> bus_name_to_bus_;
	std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_; // Остановки и маршруты
};

} // namespace transport

using transport::TransportCatalogue; // Делаем данный класс доступным в глобальном namespace