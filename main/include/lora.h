#pragma once
#include <HardwareSerial.h>


namespace lora
{

void init();
void send(String message);
void lora_log(void* pvParameters);
bool is_available();

} // namespace lora