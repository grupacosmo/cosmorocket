#include "LoRa.hpp"

// Second serial port on ESP32

int i = 1;

void setup() {
  Serial.begin(115200);
  Serial.println("start");
  Lora::init();
  // use lora2 ns for debug
}

void loop() {
  Lora::send(give_mess(i));
  delay(5000);  // delay for readable sake
  i++;
}

String give_mess(int i) { return String(i) + ". Wiadomosc"; }
