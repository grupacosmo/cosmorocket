
#ifndef LORA_HPP
#define LORA_HPP

#include <HardwareSerial.h>

namespace Lora {
extern HardwareSerial LoRaWioE5;

void init();
void send(String wiadomosc);

}  // namespace Lora

#endif
