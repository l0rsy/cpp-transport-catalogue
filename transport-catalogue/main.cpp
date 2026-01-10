#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>

int main() {
    // Устанавливаем локаль для корректного вывода чисел
    std::locale::global(std::locale("C"));
    
    try {
        // Читаем весь ввод в строку
        std::ostringstream input_stream;
        input_stream << std::cin.rdbuf();
        std::string input_str = input_stream.str();
        
        // Создаем ридер из строки
        json_reader::JsonReader reader(input_str);
        
        // Загружаем данные в каталог
        reader.LoadData();  // В этом методе теперь также строится граф
        
        // Обрабатываем запросы и получаем JSON документ
        json::Document responses = reader.ProcessRequests();
        
        // Выводим результат
        json::Print(responses, std::cout);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}