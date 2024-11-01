#pragma once
#include <HardwareSerial.h>

namespace gps {

struct Time {
  std::uint8_t hours{};
  std::uint8_t minutes{};
  std::uint8_t seconds{};
};

struct Data {
  Time time{};
  double lat{};
  double lng{};
};

void init();
void gps_task(void *pvParameters);

}  // namespace gps
