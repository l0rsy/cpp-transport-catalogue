#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include <iostream>
#include <fstream>
#include <locale>

int main() {
    // Устанавливаем локаль для корректного вывода чисел
    std::locale::global(std::locale("C"));
    
    transport::TransportCatalogue catalogue;
    json_reader::JsonReader reader(catalogue, std::cin);
    
    try {
        // Загружаем данные (остановки и маршруты)
        reader.LoadData();
        
        // Обрабатываем запросы и выводим результат в JSON
        reader.ProcessRequests(std::cout);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}