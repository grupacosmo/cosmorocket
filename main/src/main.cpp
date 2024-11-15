#include <Arduino.h>
#include <Wire.h>
#include <freertos/message_buffer.h>

#include "bmp.h"
#include "board_config.h"
#include "gps.h"
#include "led.h"
#include "lora.h"
#include "memory.h"
#include "mpu.h"

struct Global {
  Adafruit_BMP280 bmp;
  TinyGPSPlus gps;
  HardwareSerial gps_serial{gps::SERIAL_NUM};
  HardwareSerial lora_serial{1};
  MessageBufferHandle_t message_buffer;
};

Global global;

void read_data(void* /* params */);
void send_data(void* /* params */);

void setup() {
  global.message_buffer = xMessageBufferCreate(128);

  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(BAUD_RATE);
  Serial.println("--- ROCKET COMPUTER START ---");

  memory::init();
  gps::init(global.gps_serial);
  bmp::init(global.bmp);
  lora::init(global.lora_serial);
  // mpu::init(global.mpu);

  xTaskCreate(led::blink_task, "blink", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  xTaskCreate(read_data, "read_data", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);
  xTaskCreate(send_data, "send_data", DEFAULT_TASK_SIZE, nullptr, 1, nullptr);

  memory::config = memory::Config{222, 456.78, "Hello, EEPROM2!"};
  memory::save_config();
#ifdef DEBUG
  memory::print_debug();
#endif
}

void loop() {}

struct Data {
  bmp::Data bmp;
  gps::Data gps;
  mpu::Data mpu;
};

void read_data(void* /* params */) {
  Data data;
  data.bmp = bmp::read(global.bmp);
  gps::read(global.gps, global.gps_serial, data.gps);
  // TODO: read mpu
  
  const TickType_t timeout = pdMS_TO_TICKS(100);
  xMessageBufferSend(&global.message_buffer, (uint8_t*)&data, sizeof(Data), timeout);
}

void send_data(void* /* params */) {
  Data data;
  const TickType_t timeout = pdMS_TO_TICKS(100);
  size_t nread = xMessageBufferReceive(&global.message_buffer, (uint8_t*)&data, sizeof(Data), timeout);
  assert(nread == sizeof(Data));

  // TODO: serialize data and send
  lora::send(global.lora_serial, "");
}