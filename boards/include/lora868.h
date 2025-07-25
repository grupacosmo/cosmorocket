#pragma once
#include <Arduino.h>

// DO NOT RENAME THIS FILE, IT HAD A CONFLICT WITH LORA.H LIBRARY

namespace lora {

void init();
void lora_log(const String &message);
bool is_packet_received();
String get_received_message();
void clear_packet_flag();
// String lora_read();

}  // namespace lora