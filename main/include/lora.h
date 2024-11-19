#pragma once
#include <HardwareSerial.h>

namespace lora {

void init(HardwareSerial& lora_serial);
void send(HardwareSerial& lora_serial, const String &message);
bool check_availability(HardwareSerial& lora_serial);

}  // namespace lora