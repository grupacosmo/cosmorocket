#include "gps.h"
#include "TinyGPS++.h"
#include <cstdint>

namespace gps {

// Private
namespace {
    constexpr std::uint8_t BAUD_RATE = 9600;
    constexpr std::uint8_t RX_PIN = 34;
    constexpr std::uint8_t TX_PIN = 12;
    constexpr std::uint8_t SERIAL_NUM = 1;

    TinyGPSPlus tiny_gps;

    void print_debug(const Data& gps_data) {
        Serial.printf("Lat: %.6f Long: %.6f Time: %02d:%02d:%02d\n",
            gps_data.lat, gps_data.lng, gps_data.time.hours,
            gps_data.time.minutes, gps_data.time.seconds);
    }
} // namespace

HardwareSerial GPSSerial(SERIAL_NUM);

void init() {
    GPSSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
}

void gps_task([[maybe_unused]] void *pvParameters) {
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
#ifdef DEBUG
                print_debug(&gps_data);
#endif
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

} // namespace gps
