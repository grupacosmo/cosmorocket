#pragma once
#include <RadioLib.h>
#define RADIO_BOARD_TTGO_LORA32_V1_V21
#include <RadioBoards.h>
namespace lora {

void init();
void lora_log(String str);

}  // namespace lora