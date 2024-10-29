#pragma once

#include <cstdint>
#ifdef TBEAM

constexpr std::int8_t SD_SCK = 25;
constexpr std::int8_t SD_MISO = 15;
constexpr std::int8_t SD_MOSI = 32;
constexpr std::int8_t SD_SS = 33;

constexpr std::uint8_t SDA_PIN = 21;
constexpr std::uint8_t SCL_PIN = 22;

#else

constexpr std::int8_t SD_SCK = 18;
constexpr std::int8_t SD_MISO = 19;
constexpr std::int8_t SD_MOSI = 23;
constexpr std::int8_t SD_SS = 5;

constexpr std::uint8_t SDA_PIN = 13;
constexpr std::uint8_t SCL_PIN = 15;

#endif

constexpr unsigned long BAUD_RATE = 115200;
constexpr std::uint32_t DEFAULT_TASK_SIZE = 10000;
