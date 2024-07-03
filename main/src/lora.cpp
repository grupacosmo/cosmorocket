#include "lora.h"

std::uint8_t constexpr TX_pin = 15; //must rename because of conflict with one of libs
std::uint8_t constexpr RX_pin = 19;
constexpr std::uint16_t frequency = 868;
constexpr char spreadingFactor[] = "SF12";
constexpr std::uint8_t bandwidth = 125;
constexpr std::uint8_t TXPR = 12;
constexpr std::uint8_t RXPR = 15;
constexpr std::uint8_t power = 14;
constexpr char crc[] = "ON";
constexpr char IQ[] = "OFF";
constexpr char NET[] = "OFF";


namespace lora {
//https://files.seeedstudio.com/products/317990687/res/LoRa-E5+AT+Command+Specification_V1.0+.pdf

HardwareSerial LoRaWioE5(1);

//function checking lora's availability
bool is_lora_available() {
  LoRaWioE5.println("AT");

  delay(1000);

  if (LoRaWioE5.find("OK")) {
    Serial.println("LoRa module is available");
    return true;
  }

  Serial.println("LoRa module is not available");
  return false;
}

void init() {
  // communication with LoRa Wio E5, RX and TX pins, baudrate 9600
  String atCommand = "AT+MODE=TEST\r\n";
  LoRaWioE5.begin(9600, SERIAL_8N1, TX_pin, RX_pin);
  LoRaWioE5.print(atCommand);  // set transmitter in test mode
  delay(1000);

  // check if lora is available
  is_lora_available();

  // setup LoRa for transmitting
  atCommand = String("AT+TEST=RFCFG, ") + frequency + ", " + spreadingFactor + ", " + bandwidth + ", " + TXPR + ", " + RXPR + ", " + power + ", " + crc + ", " + IQ + ", " + NET + "\r\n";

  LoRaWioE5.print(atCommand);
  delay(1000);
}

//function for testing sake
String gen_rand() {
  String result = "";
  srand(time(0));  
  for (int i = 0; i < 17; i++) {
    int random_number =
        rand() % 1001;  
    result += String(random_number); 
    result +=
        ", ";  // comma added to get needed format of string
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

}  // namespace lora
