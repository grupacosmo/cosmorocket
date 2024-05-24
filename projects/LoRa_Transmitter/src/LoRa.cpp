#include "LoRa.hpp"

namespace Lora {

HardwareSerial LoRaWioE5(1);

void init() {
  // communication with LoRa Wio E5, RX and TX pins, baudrate 9600
  LoRaWioE5.begin(9600, SERIAL_8N1, 15, 19);
  LoRaWioE5.print("AT+MODE=TEST\r\n");  // set transmitter in test mode

  // check if lora is available
  if (LoRaWioE5.readString() != "+MODE: TEST") {
    Serial.println("\nLora is not available");
  }
  delay(1000);

  // setup LoRa for transmitting
  LoRaWioE5.print(
      "AT+TEST=RFCFG, 868, SF12, 125, 12, 15, 14, ON, OFF, OFF\r\n");
  delay(1000);
}

void send(String wiadomosc) {
  LoRaWioE5.print("AT+TEST=TXLRSTR,\"" + wiadomosc + "\"\r\n");
  // carriage return is essential
}

}  // namespace Lora