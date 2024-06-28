#include "gps.h"
#include "TinyGPS++.h"
#include <cstdint>

TinyGPSPlus tiny_gps;
gps::Data gps_data;
bool data_available = false;

uint8_t const GPS_SERIAL_NUM = 1;
uint8_t const GPS_RX_PIN = 34;
uint8_t const GPS_TX_PIN = 12;

namespace gps {
HardwareSerial GPSSerial(GPS_SERIAL_NUM);

void init() { GPSSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); }

bool data_is_available() { return data_available; }
Data get_gps_data() { return gps_data; }

void gps_task(void *pvParameters) {
  for (;;) {
    while (GPSSerial.available()) {
      bool sentence_finished = tiny_gps.encode(GPSSerial.read());

      if (sentence_finished && tiny_gps.location.isValid() &&
          tiny_gps.time.isValid()) {
        gps_data.lat = tiny_gps.location.lat();
        gps_data.lng = tiny_gps.location.lng();
        gps_data.time.hours = tiny_gps.time.hour();
        gps_data.time.minutes = tiny_gps.time.minute();
        gps_data.time.seconds = tiny_gps.time.second();

        data_available = true;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
} // namespace gps
