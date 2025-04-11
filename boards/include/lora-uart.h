#pragma once
#include <HardwareSerial.h>

#include "logger.h"
namespace lora {

void init();
void lora_log(String &message);
bool is_available();

}  // namespace lora