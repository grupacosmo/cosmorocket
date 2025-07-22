#include "memory.h"

#include <SPIFFS.h>

#include <cstring>  // For strcpy

namespace memory {

// Define the global config variable
Cfg config;

// Private
namespace {

constexpr uint8_t config_addr = 0;           // Address in EEPROM
constexpr size_t memory_size = sizeof(Cfg);  // EEPROM size
File log_file;

Cfg read_cfg_file() {
  Cfg cfg;
  File cfg_file = SPIFFS.open("/cfg", "r");
  if (!cfg_file) {
    Serial.println("Failed to open config file");
    return Cfg();
  }
  size_t read = cfg_file.read((uint8_t *)&cfg, sizeof(Cfg));
  if (read != sizeof(Cfg)) {
    Serial.println("Failed to read config file");
    return Cfg();
  }
  cfg_file.close();
  return cfg;
}

}  // namespace

void write_cfg_file(const Cfg &cfg) {
  File cfg_file = SPIFFS.open("/cfg", "w");

  size_t written = cfg_file.write((uint8_t *)&cfg, sizeof(Cfg));
  if (written != 1) {
    Serial.println("Failed to write config file");
  }
  cfg_file.close();
}

void init() {
  SPIFFS.begin(true);
  config = read_cfg_file();

  log_file = SPIFFS.open("/data.log", "a");
  if (!log_file) {
    Serial.println("Failed to open file for writing");
  }
}
void print_debug() {
  Serial.printf(
      "[Config Debug]:\n"
      "--------- Status: %d, Launch Time: %lu\n"
      "--------- Accel Cal: [%.2f, %.2f, %.2f]\n"
      "--------- Gyro Cal: [%.2f, %.2f, %.2f]\n"
      "--------- Pressure Cal: %.2f, Launch Alt: %.2f\n"
      "--------- Primary Chute Alt: %d\n",
      config.status, config.launch_time, config.accel_calibration[0],
      config.accel_calibration[1], config.accel_calibration[2],
      config.gyro_calibration[0], config.gyro_calibration[1],
      config.gyro_calibration[2], config.pressure_calibration,
      config.launch_altitude, config.first_parachute_height_log);
}

void save_data(String &log) { log_file.println(log); }

}  // namespace memory