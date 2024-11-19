#pragma once
#include <HardwareSerial.h>
#include <TinyGPS++.h>

namespace gps {

constexpr uint8_t SERIAL_NUM = 1;

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

void init(HardwareSerial& gps_serial);
bool read(TinyGPSPlus& gps, HardwareSerial& serial, Data& data);

}  // namespace gps
