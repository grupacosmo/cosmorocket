#pragma once
#include <Arduino.h>

// DO NOT RENAME THIS FILE, IT HAD A CONFLICT WITH LORA.H LIBRARY

namespace lora {

void init();
void lora_log(const String &message);
String lora_read();

}  // namespace lora