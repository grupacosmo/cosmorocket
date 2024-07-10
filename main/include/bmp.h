#pragma once
#include <optional>
namespace bmp {
void init();

struct Data {
    float temperature;
    float pressure;
    float altitude;

    float get_as_hpa();
};

void get_bmp(void *pvParameters);
void print_data(void *pvParameters);
std::optional<Data> measure();
void pretty_print(std::optional<Data> const &bmp_data);
} // namespace bmp
