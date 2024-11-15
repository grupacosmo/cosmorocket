#include "lora.h"

// LoRa AT commands documentation
// https://files.seeedstudio.com/products/317990687/res/LoRa-E5+AT+Command+Specification_V1.0+.pdf

namespace lora {
// Private
namespace {
constexpr uint8_t TX_PIN = 15;
constexpr uint8_t RX_PIN = 19;

constexpr uint8_t TXPR = 12;
constexpr uint8_t RXPR = 15;

constexpr uint16_t BAUD_RATE = 9600;
constexpr uint16_t FREQUENCY = 868;
constexpr uint8_t BANDWIDTH = 125;
constexpr uint8_t POWER = 14;

constexpr char SPREADING_FACTOR[] = "SF12";
constexpr char CRC[] = "ON";
constexpr char IQ[] = "OFF";
constexpr char NET[] = "OFF";

}  // namespace

bool check_availability(HardwareSerial& lora_serial) {
  lora_serial.println("AT");
  delay(1000);
  return lora_serial.find("OK");
}

void init(HardwareSerial& lora_serial) {
  String atCommand{"AT+MODE=TEST\r\n"};
  lora_serial.begin(BAUD_RATE, SERIAL_8N1, TX_PIN, RX_PIN);
  lora_serial.print(atCommand);
  delay(1000);

  if (!check_availability(lora_serial)) {
    Serial.println("LoRa module is not available");
    return;
  }

  // LoRa setup
  atCommand = String("AT+TEST=RFCFG, ") + FREQUENCY + ", " + SPREADING_FACTOR +
              ", " + BANDWIDTH + ", " + TXPR + ", " + RXPR + ", " + POWER +
              ", " + CRC + ", " + IQ + ", " + NET + "\r\n";

  lora_serial.print(atCommand);
  delay(1000);
}


void send(HardwareSerial& lora_serial, const String &message) {
  lora_serial.print("AT+TEST=TXLRSTR,\"" + message + "\"\r\n");
}

}  // namespace lora
