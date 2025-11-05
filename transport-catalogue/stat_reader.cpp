#include "stat_reader.h"

#include <iostream>
#include <iomanip>

using namespace std;

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, string_view request,
                       ostream& output) {
    auto space_pos = request.find(' ');
    if (space_pos == request.npos) {
        return;
    }
    
    string_view command = request.substr(0, space_pos);
    string_view argument = Trim(request.substr(space_pos + 1));
    // Запрос информации об маршруте
    if (command == "Bus") {
        string bus_name(argument);
        auto bus_info = transport_catalogue.GetBusInfo(bus_name);
        
        output << "Bus " << bus_name << ": ";
        if (bus_info) {
            output << bus_info->stops_count << " stops on route, "
                   << bus_info->unique_stops_count << " unique stops, "
                   << static_cast<int>(bus_info->route_length) << " route length, "
                   << fixed << setprecision(6) << bus_info->curvature << " curvature"; 
        } else {
            output << "not found";
        }
        output << endl;
    }
    // Запрос информации об остановке
    else if (command == "Stop") {
        string stop_name(argument);
        auto stop_info = transport_catalogue.GetStopInfo(stop_name);
        
        output << "Stop " << stop_name << ": ";
        if (!stop_info) {
            output << "not found";
        } else if (stop_info->buses.empty()) {
            output << "no buses";
        } else {
            output << "buses";
            for (const auto& bus_name : stop_info->buses) {
                output << " " << bus_name;
            }
        }
        output << endl;
    }
}

// Вспомогательная функция для обрезки пробелов
inline string_view Trim(string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}