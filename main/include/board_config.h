#pragma once

#include <cstdint>
#ifdef TBEAM

int8_t constexpr SD_SCK = 25;
int8_t constexpr SD_MISO = 15;
int8_t constexpr SD_MOSI = 32;
int8_t constexpr SD_SS = 33;

uint8_t constexpr SDA_PIN = 21;
uint8_t constexpr SCL_PIN = 22;

#else

int8_t constexpr SD_SCK = 18;
int8_t constexpr SD_MISO = 19;
int8_t constexpr SD_MOSI = 23;
int8_t constexpr SD_SS = 5;

uint8_t constexpr SDA_PIN = 13;
uint8_t constexpr SCL_PIN = 15;

#endif

unsigned long constexpr BAUD_RATE = 115200;
std::uint32_t constexpr DEFAULT_TASK_SIZE = 10000;
