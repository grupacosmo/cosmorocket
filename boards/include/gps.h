#pragma once
#include <HardwareSerial.h>

namespace gps {

struct Time {
  uint8_t hours{};
  uint8_t minutes{};
  uint8_t seconds{};
};

struct Data {
  Time time{};
  double lat{};
  double lng{};
};

void init();
Data get_data();
void gps_task(void *pvParameters);
void print_data();

}  // namespace gps
