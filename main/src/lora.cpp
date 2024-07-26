#include "lora.h"

std::uint8_t constexpr TX_pin = 15; //must rename because of conflict with one of libs
std::uint8_t constexpr RX_pin = 19;
std::uint16_t constexpr frequency = 868;
char constexpr spreadingFactor[] = "SF12";
std::uint8_t constexpr bandwidth = 125;
std::uint8_t constexpr TXPR = 12;
std::uint8_t constexpr RXPR = 15;
std::uint8_t constexpr power = 14;
char constexpr crc[] = "ON";
char constexpr IQ[] = "OFF";
char constexpr NET[] = "OFF";
String log_message;


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

  srand(time(0));  
  // check if lora is available
  is_lora_available();

  // setup LoRa for transmitting
  //atCommand = String("AT+TEST=RFCFG, ") + frequency + ", " + spreadingFactor + ", " + bandwidth + ", " + TXPR + ", " + RXPR + ", " + power + ", " + crc + ", " + IQ + ", " + NET;
  atCommand = "AT+TEST=RFCFG, 868, SF11, 500, 12, 15, 14, ON, OFF, OFF\r\n";
  Serial.println(atCommand);

  LoRaWioE5.print(atCommand);
  delay(1000);
}

//function for testing sake

String gen_rand() {
  String result = "";
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
  
  LoRaWioE5.print("AT+TEST=TXLRSTR, \"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"\r\n");//LoRaWioE5.print("AT+TEST=TXLRSTR, \"" + message + "\"\r\n");
  // carriage return is essential
}

void lora_log(void* pvParameters){
  for(;;){
    log_message = gen_rand();
    String send_message = log_message;//;
    send("ABCabc");
    //Serial.println(log_message);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}

}  // namespace lora
