#pragma once
#include <cmath>
namespace bmp {
void init();

struct Data {
    float temperature{NAN};
    float pressure{NAN};
    float altitude{NAN};

    float hpa() const { return pressure / 100.f; }
};

void get_bmp(void *pvParameters);
void print_data(void *pvParameters);
void pretty_print();
Data get_data();
} // namespace bmp
