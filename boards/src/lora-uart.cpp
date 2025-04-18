#ifndef TBEAM
#include "lora-uart.h"

// LoRa AT commands documentation
// https://files.seeedstudio.com/products/317990687/res/LoRa-E5+AT+Command+Specification_V1.0+.pdf

namespace lora {

HardwareSerial LoRaWioE5(1);
// Private
namespace {
constexpr uint8_t TX_PIN = 32;
constexpr uint8_t RX_PIN = 33;

constexpr uint8_t TXPR = 12;
constexpr uint8_t RXPR = 15;

constexpr uint16_t BAUD_RATE = 9600;
constexpr uint16_t FREQUENCY = 868;
constexpr uint8_t BANDWIDTH = 125;
constexpr uint8_t POWER = 22;

constexpr char SPREADING_FACTOR[] = "SF12";
constexpr char CRC[] = "ON";
constexpr char IQ[] = "OFF";
constexpr char NET[] = "OFF";

}  // namespace

bool check_availability() {
  LoRaWioE5.println("AT");
  delay(1000);
  return LoRaWioE5.find("OK");
}

void init() {
  String atCommand{"AT+MODE=TEST\r\n"};
  LoRaWioE5.begin(BAUD_RATE, SERIAL_8N1, TX_PIN, RX_PIN);
  LoRaWioE5.print(atCommand);
  delay(1000);

  if (!check_availability()) {
    Serial.println("LoRa module is not available");
    return;
  }

  // LoRa setup
  atCommand = String("AT+TEST=RFCFG, ") + FREQUENCY + ", " + SPREADING_FACTOR +
              ", " + BANDWIDTH + ", " + TXPR + ", " + RXPR + ", " + POWER +
              ", " + CRC + ", " + IQ + ", " + NET + "\r\n";

  LoRaWioE5.print(atCommand);
  delay(1000);
}

void lora_log(String &message) {
  LoRaWioE5.print("AT+TEST=TXLRSTR,\"" + message + "\"\r\n");
}

}  // namespace lora

#endif  // TBEAM
