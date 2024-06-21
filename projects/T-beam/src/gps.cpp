#include "gps.h"

namespace gps {

HardwareSerial GPSSerial(GPS_SERIAL_NUM);

void init() { GPSSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); }

void gps_loop(void* pvParameters) {
  String nmea = "";
  for (;;) {
    while (GPSSerial.available()) {
      char c = GPSSerial.read();
      nmea += c;
    }

    if (nmea != "") {
      Serial.println(nmea);
      nmea = "";
    } else
      Serial.println("------------");
    vTaskDelay(pdMS_TO_TICKS(900));

  }
}

}  // namespace gps
