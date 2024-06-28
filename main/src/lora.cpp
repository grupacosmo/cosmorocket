#include "lora.h"
#define TX 15
#define RX 19

namespace lora {

HardwareSerial LoRaWioE5(1);

void init() {
  // communication with LoRa Wio E5, RX and TX pins, baudrate 9600
  LoRaWioE5.begin(9600, SERIAL_8N1, TX, RX);
  LoRaWioE5.print("AT+MODE=TEST\r\n");  // set transmitter in test mode
  delay(1000);

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

String gen_rand() {
  String result = "";
  srand(time(0));  // inicjalizacja generatora liczb losowych
  for (int i = 0; i < 17; i++) {
    int random_number =
        rand() % 1001;  // generowanie losowej liczby z zakresu 0-1000
    result += String(random_number);  // Konwertuje liczbę na ciąg znaków
    result +=
        ", ";  // Dodaje przecinek po każdej liczbie, z wyjątkiem ostatniej
  }
  return result;
}

void send(String message) {
  LoRaWioE5.print("AT+TEST=TXLRSTR,\"" + message + "\"\r\n");
  // carriage return is essential
}


void lora_log(void* pvParameters){
  for(;;){
    String send_message = log_message;
    send(send_message);
    vTaskDelay(pdMS_TO_TICKS(500));
  }

}

}  // namespace Lora
