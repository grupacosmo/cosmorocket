#pragma once
#include <optional>
namespace bmp {
void init();

struct Data {
    float temperature;
    float pressure;
    float altitude;

    static float get_as_hpa();
};

void get_bmp(void *pvParameters);
void print_data(void *pvParameters);
void pretty_print(std::optional<Data> const &bmp_data);
std::optional<Data> get_data();
} // namespace bmp
