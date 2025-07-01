#pragma once
#include <cstdint>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "mpu.h"

namespace logger {

struct Packet {
  bmp::Data bmp_data{};
  mpu::Data mpu_data{};
  gps::Data gps_data{};
  uint32_t sys_time{};
  uint16_t n{};
  uint8_t status{};
};

const char *serialize(Packet &packet);

}  // namespace logger
