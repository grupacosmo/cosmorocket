#pragma once

#include <cstdint>
#ifdef TBEAM

constexpr int8_t SD_SCK = 25;
constexpr int8_t SD_MISO = 15;
constexpr int8_t SD_MOSI = 32;
constexpr int8_t SD_SS = 33;

constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;

#else

constexpr int8_t SD_SCK = 18;
constexpr int8_t SD_MISO = 19;
constexpr int8_t SD_MOSI = 23;
constexpr int8_t SD_SS = 5;

constexpr uint8_t SDA_PIN = 13;
constexpr uint8_t SCL_PIN = 15;

#endif

constexpr unsigned long BAUD_RATE = 115200;
constexpr uint32_t SIZE = 10000;
constexpr size_t SERIALIZE_PACKET_SIZE = 350;
