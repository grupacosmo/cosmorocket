#pragma once
#include <cmath>

namespace bmp {

struct Data {
    float temperature{};
    float pressure{};
    float altitude{};
};

void init();
void get_bmp(void *pvParameters);
void print_data(void *pvParameters);

} // namespace bmp
