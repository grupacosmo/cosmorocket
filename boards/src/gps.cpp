#include "gps.h"

#include <cstdint>

#include "TinyGPS++.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace gps {

// Private
namespace {
constexpr uint16_t BAUD_RATE = 9600;
constexpr uint8_t RX_PIN = 34;
constexpr uint8_t TX_PIN = 12;
constexpr uint8_t SERIAL_NUM = 1;
constexpr uint16_t MAX_BYTES_PER_CYCLE = 256;

Data gps_data;

TinyGPSPlus tiny_gps;
SemaphoreHandle_t gps_mutex;

}  // namespace

HardwareSerial GPSSerial(SERIAL_NUM);

void init() {
  gps_mutex = xSemaphoreCreateMutex();
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
  Data temp_data;
  if (xSemaphoreTake(gps_mutex, pdMS_TO_TICKS(10))) {
    temp_data = gps_data;
    xSemaphoreGive(gps_mutex);
  } else {
    Serial.println("Warning: Failed to get GPS mutex in get_data()");
  }
  return temp_data;
}

void gps_task([[maybe_unused]] void* pvParameters) {
  for (;;) {
    uint16_t bytes_processed = 0;

    while (bytes_processed < MAX_BYTES_PER_CYCLE && GPSSerial.available()) {
      bytes_processed++;
      bool msg_finished = tiny_gps.encode(GPSSerial.read());

      if (msg_finished && tiny_gps.location.isValid() &&
          tiny_gps.time.isValid()) {
        if (xSemaphoreTake(gps_mutex, pdMS_TO_TICKS(10))) {
          gps_data.lat = tiny_gps.location.lat();
          gps_data.lng = tiny_gps.location.lng();
          gps_data.time.hours = tiny_gps.time.hour();
          gps_data.time.minutes = tiny_gps.time.minute();
          gps_data.time.seconds = tiny_gps.time.second();
          xSemaphoreGive(gps_mutex);
        } else {
          Serial.println("Warning: Failed to get GPS mutex in gps_task()");
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

}  // namespace gps
