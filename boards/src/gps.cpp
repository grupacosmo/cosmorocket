#include "gps.h"

#include <cstdint>

#include "TinyGPS++.h"

namespace gps {

// Private
namespace {
constexpr uint16_t BAUD_RATE = 9600;
constexpr uint8_t RX_PIN = 34;
constexpr uint8_t TX_PIN = 12;
constexpr uint8_t SERIAL_NUM = 1;
Data gps_data;
TinyGPSPlus tiny_gps;

}  // namespace

HardwareSerial GPSSerial(SERIAL_NUM);

void init() {
  GPSSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("GPS initialized successfully.");
}

void print_data() {
  Serial.print("[GPS]: ");
  Serial.print(gps_data.lat, 6);
  Serial.print(", ");
  Serial.print(gps_data.lng, 6);
  Serial.print(" | ");
  Serial.print(gps_data.time.hours);
  Serial.print(":");
  Serial.print(gps_data.time.minutes);
  Serial.print(":");
  Serial.println(gps_data.time.seconds);
}

Data get_data() {
#ifdef DEBUG
  print_data();
#endif
  return gps_data;
}

void gps_task([[maybe_unused]] void* pvParameters) {
  for (;;) {
    while (GPSSerial.available()) {
      bool msg_finished = tiny_gps.encode(GPSSerial.read());

      if (msg_finished && tiny_gps.location.isValid() &&
          tiny_gps.time.isValid()) {
        gps_data.lat = tiny_gps.location.lat();
        gps_data.lng = tiny_gps.location.lng();
        gps_data.time.hours = tiny_gps.time.hour();
        gps_data.time.minutes = tiny_gps.time.minute();
        gps_data.time.seconds = tiny_gps.time.second();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

}  // namespace gps
