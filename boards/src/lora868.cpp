#ifdef TBEAM
#include "lora868.h"

#include <LoRa.h>  // https://github.com/sandeepmistry/arduino-LoRa
#include <SPI.h>

namespace lora {

// We are overwriting the library pins here don't yell at me

#define LORA_SCK 5       // GPIO5 - SX1276 SCK
#define LORA_MISO 19     // GPIO19 - SX1276 MISO
#define LORA_MOSI 27     // GPIO27 - SX1276 MOSI
#define LORA_CS 18       // GPIO18 - SX1276 CS
#define LORA_RST_PIN 14  // GPIO14 - SX1276 RST
#define LORA_IRQ 26      // GPIO26 - SX1276 IRQ (interrupt request)

constexpr int spreading_factor = 12;  // Ranges from 6-12, default is 7
constexpr int bandwidth = 125E3;
constexpr int tx_power = 1;  // Set transmission power (default 1,  flight 22)
constexpr int frequency = 868E6;  // Set frequency to 868 MHz

void init() {
  Serial.print("[SX1262] Initializing ... ");
  LoRa.setPins(LORA_CS, LORA_RST_PIN, LORA_IRQ);

  if (!LoRa.begin(frequency)) {
    Serial.println("Starting LoRa failed!");
    while (1);  // If LoRa won't start, enter infinite loop
  }

  LoRa.setSpreadingFactor(spreading_factor);
  LoRa.setTxPower(tx_power, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSignalBandwidth(bandwidth);

  Serial.println("LoRa initialized successfully.");
}

void lora_log(const String &message) {
  Serial.println(message);
  LoRa.beginPacket();
  LoRa.print(message);
  if (LoRa.endPacket() == 0) {
    Serial.println("[ERROR] Failed to send LoRa packet!");
  }
}

String lora_read() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }
    return message.c_str();
  }
  return "";
}

}  // namespace lora
#endif  // TBEAM