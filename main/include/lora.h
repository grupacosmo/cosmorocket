#ifndef LORA_H
#define LORA_H
#include <HardwareSerial.h>


namespace lora
{
    extern HardwareSerial LoRaWioE5;
    inline String log_message;

    void init();
    void send(String wiadomosc);
    String gen_rand();
    void lora_log(void* pvParameters);


} // namespace Lora

#endif