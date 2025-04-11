#pragma once
#include <RadioLib.h>
#define RADIO_BOARD_TTGO_LORA32_V1_V21
#include <RadioBoards.h>

#include "logger.h"
namespace lora {

void init();
void lora_log(String &message);

}  // namespace lora