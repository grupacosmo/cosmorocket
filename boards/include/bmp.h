#pragma once
#include <cmath>

namespace bmp {

struct Data {
  float temperature{};
  float pressure{};
  float altitude{};
};

void init();
Data get_data();
void print_data(void *pvParameters);

}  // namespace bmp
