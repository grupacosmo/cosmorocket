#include "gps.h"
#include "TinyGPS++.h"
#include <cstdint>

constexpr uint8_t GPS_SERIAL_NUM = 1;
constexpr uint8_t GPS_RX_PIN = 34;
constexpr uint8_t GPS_TX_PIN = 12;

namespace {
TinyGPSPlus tiny_gps;
bool data_available = false;

gps::Data gps_data;
} // namespace

namespace gps {
HardwareSerial GPSSerial(GPS_SERIAL_NUM);

void init() { GPSSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); }

bool data_is_available() { return data_available; }
Data get_data() { return gps_data; }

void gps_task([[maybe_unused]] void *pvParameters) {
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

void print_data([[maybe_unused]] void *pvParameters) {
    for (;;) {
        if (!data_is_available()) {
            Serial.println("GPS: no fix");
        } else {
            gps::Data gps_data = get_data();
            Serial.printf("Lat: %.6f Long: %.6f Time: %02d:%02d:%02d\n",
                          gps_data.lat, gps_data.lng, gps_data.time.hours,
                          gps_data.time.minutes, gps_data.time.seconds);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

} // namespace gps
