#pragma once
#include <HardwareSerial.h>


namespace lora
{
    extern HardwareSerial LoRaWioE5;

    void init();
    void send(String message);
    String gen_rand();
    void lora_log(void* pvParameters);
    bool is_lora_available();

} // namespace lora
