#include <Arduino.h>
#include <Wire.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "ignition.h"
#include "logger.h"
#include "memory.h"
#include "mpu.h"

#ifdef TBEAM
#include "lora868.h"
#else
#include "lora-uart.h"
#endif

// TaskHandle_t camPtr = nullptr;

void flight_controller(const logger::Packet &packet);
void main_task_loop(void *pvParameters);
void cam_task(void *pvParameters);

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD_RATE);
  Serial.println("--- ROCKET COMPUTER START ---");

  pinMode(BUTTON, INPUT_PULLUP);

  memory::init();
  gps::init();
  bmp::init();
  mpu::init();
  ignition::init();
  lora::init();

  // Pin mpu_task to core 0
  xTaskCreatePinnedToCore(mpu::mpu_task, "mpu", 64000, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(gps::gps_task, "gps", 64000, nullptr, 1, nullptr, 0);
  // Pin main_task_loop to core 1
  xTaskCreatePinnedToCore(main_task_loop, "main", 16384, nullptr, 1, nullptr,
                          1);

  Serial.println("--- SETUP FINISHED. ---");
}

void loop() {}

// sperate task pinned to core 1
void main_task_loop(void *pvParameters) {
  logger::Packet packet;
  while (true) {
    packet.bmp_data = bmp::get_data();
    packet.mpu_data = mpu::get_data();
    // packet.gps_data = gps::get_data();
    flight_controller(packet);
    if (memory::config.status != memory::DEV) {
      String message = logger::serialize(packet);
      lora::lora_log(message);
      memory::save_data(message);
    }
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

// First draft of the flight controller logic
void flight_controller(const logger::Packet &packet) {
  static float last_altitude = 0.0;
  float rel_alt = packet.bmp_data.altitude - memory::config.launch_altitude;

  switch (memory::config.status) {
    case memory::DEV:
      if (lora::is_packet_received()) {
        String msg = lora::get_received_message();
        lora::clear_packet_flag();

        if (msg == "LAUNCH") {
          Serial.println("Launch command received!");
          memory::config.launch_altitude = packet.bmp_data.altitude;
          memory::config.status = memory::PRE_LAUNCH;
          memory::write_cfg_file(memory::config);
        }
      }
      // alternative way to switch to PRE_LAUNCH mode using builtin button
      if (digitalRead(BUTTON) == LOW) {
        Serial.println("Button pressed, switching to PRE_LAUNCH mode.");
        memory::config.launch_altitude = packet.bmp_data.altitude;
        memory::config.status = memory::PRE_LAUNCH;
        memory::write_cfg_file(memory::config);
        vTaskDelay(pdMS_TO_TICKS(1000));  // Debounce delay
      }
      break;
    case memory::PRE_LAUNCH:
      if (ignition::camPtr == nullptr) {
        ignition::duration = 120000;
        xTaskCreatePinnedToCore(ignition::cam_task, "cam", 1024, nullptr, 1,
                                &ignition::camPtr, 1);
        Serial.println("[Camera Task] Task started.");
      }

      if (rel_alt > 5.0) {
        memory::config.status = memory::ASCENT;
        memory::write_cfg_file(memory::config);
      }
      break;

    case memory::ASCENT:
      static int apogee_counter = 0;

      if (rel_alt < last_altitude) {
        apogee_counter++;
      } else {
        apogee_counter = 0;
      }
      if (apogee_counter >= 5) {
        Serial.printf("Apogee detected at %.2f meters\n", last_altitude);
        memory::config.first_parachute_height_log =
            static_cast<int>(last_altitude);

        vTaskDelay(pdMS_TO_TICKS(100));
        ignition::fire(P2_PARACHUTE);  // Fire the parachute

        memory::config.status = memory::DESCENT_PRIMARY;
        memory::write_cfg_file(memory::config);
      }
      break;

    case memory::DESCENT_PRIMARY:
      static int landing_counter = 0;

      if (std::abs(rel_alt - last_altitude) < 1.0) {
        landing_counter++;
      } else {
        landing_counter = 0;
      }

      if (landing_counter > 50) {
        memory::config.status = memory::RECOVERY;
        memory::write_cfg_file(memory::config);
      }
      break;

    case memory::RECOVERY:
      digitalWrite(P1_CAMERA, LOW);
      break;

    default:
      Serial.printf("Unknown flight status: %d\n", memory::config.status);
      break;
  }
  last_altitude = rel_alt;
}
