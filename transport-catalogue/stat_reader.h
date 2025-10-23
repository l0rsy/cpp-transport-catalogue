#pragma once

#include <string_view>

#include "transport_catalogue.h"

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);

inline std::string_view Trim(std::string_view string);