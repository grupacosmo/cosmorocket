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
File file;

}  // namespace

// Define the default initialization function
void set_defaults(Cfg &config) {
  config.flight_status = DEV;
  config.launch_time = 0;

  config.accel_calibration[0] = 0.0f;
  config.accel_calibration[1] = 0.0f;
  config.accel_calibration[2] = 0.0f;

  config.gyro_calibration[0] = 0.0f;
  config.gyro_calibration[1] = 0.0f;
  config.gyro_calibration[2] = 0.0f;

  config.pressure_calibration = 0.0f;

  config.launch_altitude = 0.0f;

  config.first_parachute_height_log = 500;
  config.second_parachute_target = 200;

  config.error_code = error_code_t::NO_ERROR;
  strcpy(config.last_error, "No errors");
}

void init() {
  EEPROM.begin(memory_size);
  if (memory_size > EEPROM.length()) {
    Serial.printf("ERROR Config size: %d, EEPROM size: %d\n", memory_size,
                  EEPROM.length());

    for (;;);
  }

  // Load configuration from EEPROM
  load_config();
  // If the configuration is invalid (e.g., uninitialized), set defaults
  if (config.flight_status == -1) {
    Serial.println("No valid configuration found. Setting defaults...");
    set_defaults(config);
    save_config();
  }
  SPIFFS.begin(true);
  file = SPIFFS.open("/data.log", "a");
  if (!file) {
    Serial.println("Failed to open file for writing");
    for (;;);
  }
}

void save_config() {
  EEPROM.put(config_addr, config);
  EEPROM.commit();
}

void load_config() { EEPROM.get(config_addr, config); }

void print_debug() {
  Serial.printf(
      "[Config Debug]:\n"
      "--------- Status: %d, Launch Time: %lu\n"
      "--------- Accel Cal: [%.2f, %.2f, %.2f]\n"
      "--------- Gyro Cal: [%.2f, %.2f, %.2f]\n"
      "--------- Pressure Cal: %.2f, Launch Alt: %.2f\n"
      "--------- Primary Chute Alt: %d, Secondary Chute Alt: %d\n"
      "--------- Error Code: %d, Last Error: %s\n",
      config.flight_status, config.launch_time, config.accel_calibration[0],
      config.accel_calibration[1], config.accel_calibration[2],
      config.gyro_calibration[0], config.gyro_calibration[1],
      config.gyro_calibration[2], config.pressure_calibration,
      config.launch_altitude, config.first_parachute_height_log,
      config.second_parachute_target, config.error_code, config.last_error);
}

void save_data(String &log) { file.println(log); }

}  // namespace memory