#include "gps.h"

#include <cstdint>

#include "TinyGPS++.h"

namespace gps {

// Private
namespace {
constexpr uint16_t BAUD_RATE = 9600;
constexpr uint8_t RX_PIN = 34;
constexpr uint8_t TX_PIN = 12;

void print_debug(const Data& gps_data) {
  Serial.printf("Lat: %.6f Long: %.6f Time: %02d:%02d:%02d\n", gps_data.lat,
                gps_data.lng, gps_data.time.hours, gps_data.time.minutes,
                gps_data.time.seconds);
}
}  // namespace

void init(HardwareSerial& gps_serial) { gps_serial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN); }

// Reads data from `gps` into `data` if a new message has been received.
bool read(TinyGPSPlus& gps, HardwareSerial& serial, Data& data) {
  // read all data from serial and encode it the gps object
  while (serial.available() > 0) {
    gps.encode(serial.read());
  }

  if (gps.location.isValid() && gps.time.isValid()) {
    data.lat = gps.location.lat();
    data.lng = gps.location.lng();
    data.time.hours = gps.time.hour();
    data.time.minutes = gps.time.minute();
    data.time.seconds = gps.time.second();
    return true;
  }

  return false;
}

}  // namespace gps
