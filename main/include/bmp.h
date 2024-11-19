#pragma once
#include <cmath>
#include <Adafruit_BMP280.h>

namespace bmp {

struct Data {
  float temperature{};
  float pressure{};
  float altitude{};
};

void init(Adafruit_BMP280& bmp);
Data read(Adafruit_BMP280& bmp);
void print_data(void *pvParameters);

}  // namespace bmp
