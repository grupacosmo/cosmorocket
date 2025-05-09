#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "logger.h"
#include "memory.h"
#include "mpu.h"

#ifdef TBEAM
#include "lora.h"
#else
#include "lora-uart.h"
#endif

void flight_controller(const logger::Packet &packet);
void main_task_loop(void *pvParameters);

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD_RATE);
  Serial.println("--- ROCKET COMPUTER START ---");

  memory::init();
  gps::init();
  bmp::init();
  mpu::init();
  lora::init();
  // Pin mpu_task to core 0
  xTaskCreatePinnedToCore(mpu::mpu_task, "mpu", SIZE, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(gps::gps_task, "gps", SIZE, nullptr, 1, nullptr, 0);
  // Pin main_task_loop to core 1
  xTaskCreatePinnedToCore(main_task_loop, "main", SIZE, nullptr, 1, nullptr, 1);

  Serial.println("--- SETUP FINISHED. ---");
}

void loop() {}

// sperate task pinned to core 1
void main_task_loop(void *pvParameters) {
  logger::Packet packet;
  for (;;) {
    packet.bmp_data = bmp::get_data();
    packet.mpu_data = mpu::get_data();
    packet.gps_data = gps::get_data();
    flight_controller(packet);
    String message = logger::serialize(packet);
    lora::lora_log(message);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// First draft of the flight controller logic
void flight_controller(const logger::Packet &packet) {
  static float last_altitude = 0.0;
  static float second_last_altitude = 0.0;
  float rel_alt = packet.bmp_data.altitude - memory::config.launch_altitude;

  switch (memory::config.flight_status) {
    case memory::PRE_LAUNCH:
      if (rel_alt > 5.0) {
        memory::config.flight_status = memory::ASCENT;
      }
      break;

    case memory::ASCENT:
      // Detect apogee when altitude stops increasing
      if (second_last_altitude > last_altitude) {
        memory::config.flight_status = memory::APOGEE;
        Serial.printf("Apogee detected at %.2f meters\n", second_last_altitude);
        memory::config.first_parachute_height_log =
            static_cast<int>(second_last_altitude);
      }
      break;

    case memory::APOGEE:
      if (rel_alt < last_altitude && second_last_altitude < last_altitude) {
        memory::config.flight_status = memory::DESCENT_PRIMARY;
      }
      break;

    case memory::DESCENT_PRIMARY:
      if (rel_alt < memory::config.second_parachute_target) {
        memory::config.flight_status = memory::DESCENT_SECONDARY;
      }
      break;

    case memory::DESCENT_SECONDARY:
      if (rel_alt < 5.0) {
        memory::config.flight_status = memory::RECOVERY;
      }
      break;

    default:
      Serial.printf("Unknown flight status: %d\n", memory::config.flight_status);
      break;
  }
  second_last_altitude = last_altitude;
  last_altitude = rel_alt;
}
