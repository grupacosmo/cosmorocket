#pragma once
#include <EEPROM.h>

namespace memory {

constexpr int error_max_length = 100;

// Flight status enumeration
enum flight_status_t {
  DEV = 0,                // Development mode
  IDLE = 1,               // System initialized, testing comms
  PRE_LAUNCH = 2,         // Waiting for launch detection
  ASCENT = 3,             // Rocket is ascending
  APOGEE = 4,             // Rocket has reached apogee
  DESCENT_PRIMARY = 5,    // Rocket is descending under the primary parachute
  DESCENT_SECONDARY = 6,  // Rocket is descending under the secondary parachute
  RECOVERY = 7,           // Rocket has landed and is ready for recovery
  ERROR = 99              // Error state
};

enum error_code_t {
  NO_ERROR = 0,  // No error
};

// Configuration struct
struct Cfg {
  flight_status_t flight_status;  // Current flight status
  unsigned long launch_time;      // Launch time in UTC timestamp
  float accel_calibration[3];     // Accelerometer calibration offsets (x, y, z)
  float gyro_calibration[3];      // Gyroscope calibration offsets (x, y, z)
  float pressure_calibration;     // Barometric pressure calibration offset
  float launch_altitude;          // Altitude at launch site (in meters)
  int first_parachute_height_log;     // Altitude for primary parachute
  int second_parachute_target;        // Altitude for secondary parachute
  error_code_t error_code;            // Error code for diagnostics
  char last_error[error_max_length];  // Description of the last error
};

// Declare the global config variable
extern Cfg config;

// Function declarations
void set_defaults(Cfg &config);
void init();
void save_config();
void load_config();
void print_debug();
void save_data(String &log);

}  // namespace memory