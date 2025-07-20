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
    packet.gps_data = gps::get_data();
    flight_controller(packet);
    if (memory::config.status != memory::DEV) {
      String message = logger::serialize(packet);
      lora::lora_log(message);  // This will now switch LoRa to transmit mode
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
        }
      }
      break;
    case memory::PRE_LAUNCH:
      if (rel_alt > 5.0) {
        memory::config.status = memory::ASCENT;
        // Maybe change this to magsaafe detected launch lol
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
        ignition::fire(1);

        memory::config.status = memory::DESCENT_PRIMARY;
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
      }
      break;
    case memory::RECOVERY:
      break;

    default:
      Serial.printf("Unknown flight status: %d\n", memory::config.status);
      break;
  }
  last_altitude = rel_alt;
}

// // #include <LoRa.h>
// // #include <SPI.h>

// // // --- Use the exact same pin definitions from your project ---
// // #define LORA_SCK 5       // GPIO5 - SX1276 SCK
// // #define LORA_MISO 19     // GPIO19 - SX1276 MISO
// // #define LORA_MOSI 27     // GPIO27 - SX1276 MOSI
// // #define LORA_CS 18       // GPIO18 - SX1276 CS
// // #define LORA_RST_PIN 14  // GPIO14 - SX1276 RST
// // #define LORA_IRQ 26      // GPIO26 - SX1276 IRQ (interrupt request)

// // --- Use the exact same LoRa parameters as your transmitter ---
// // constexpr int spreading_factor = 10;
// // constexpr int bandwidth = 250E3;
// // constexpr int frequency = 868E6;

// // void lora_task(void *pvParameters);

// // void setup() {
// //   Serial.begin(115200);
// //   while (!Serial);  // Wait for serial to connect

// //   Serial.println("--- LoRa Minimal Receiver Test ---");

// //   LoRa.setPins(LORA_CS, LORA_RST_PIN, LORA_IRQ);

// //   if (!LoRa.begin(frequency)) {
// //     Serial.println("Starting LoRa failed! Check your wiring!");
// //     while (1);  // Halt on failure
// //   }

// //   LoRa.setSpreadingFactor(spreading_factor);
// //   LoRa.setSignalBandwidth(bandwidth);

// //   // Optional: Set the same Sync Word on both sender and receiver
// //   // LoRa.setSyncWord(0xF3);

// //   Serial.println("LoRa initialized successfully. Waiting for packets...");
// //   xTaskCreatePinnedToCore(lora_task, "lora", 32768, nullptr, 1, nullptr,
// 0);
// // }

// // void lora_task([[maybe_unused]] void *pvParameters) {
// //   for (;;) {
// //     // Try to parse a packet
// //     int packetSize = LoRa.parsePacket();
// //     if (packetSize) {
// //       // Received a packet!
// //       Serial.print("Received packet '");

// //       // Read packet
// //       while (LoRa.available()) {
// //         String LoRaData = LoRa.readString();
// //         Serial.print(LoRaData);
// //       }

// //       // Print RSSI of packet
// //       Serial.print("' with RSSI ");
// //       Serial.println(LoRa.packetRssi());
// //     }
// //     vTaskDelay(pdMS_TO_TICKS(10));
// //   }
// // }
// // void loop() {}
