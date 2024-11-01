#pragma once
#include <cstdint>

#include "bmp.h"
#include "gps.h"
#include "mpu.h"

namespace logger {

struct Packet {
  bmp::Data bmp_data{};
  mpu::Data mpu_data{};
  gps::Data gps_data{};
  std::uint32_t sys_time{};
  std::uint16_t n{};
  std::uint8_t status{};
};

String serialize(Packet &packet);

}  // namespace logger
