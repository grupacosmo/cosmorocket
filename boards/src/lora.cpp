#ifdef TBEAM
#include "lora.h"
namespace lora {

Radio radio = new RadioModule();
int transmissionState = RADIOLIB_ERR_NONE;
volatile bool transmittedFlag = false;
int count = 0;

void setFlag(void) { transmittedFlag = true; }

void init() {
  // freq = 434.0 bw = 125.0 sf = 9 cr = 7 syncWord = RADIOLIB_SX127X_SYNC_WORD
  // power = 10, uint16_t preambleLength = 8, uint8_t gain = 0);

  Serial.print("[SX1278] Initializing ... ");
  int state = radio.begin();

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");

  } else {
    Serial.print("failed, code ");
    Serial.println(state);
  }
  radio.setPacketSentAction(setFlag);

  Serial.print(F("[SX1278] Sending first packet ... "));

  transmissionState = radio.startTransmit("Hello World!");
  while (!transmittedFlag) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  Serial.println("Lora initialized successfully.");
}

void lora_log(String &message) {
  if (transmittedFlag) {
    transmittedFlag = false;

    if (transmissionState != RADIOLIB_ERR_NONE) {
      Serial.print("failed, code ");
      Serial.println(transmissionState);
    }
    radio.finishTransmit();
    vTaskDelay(pdMS_TO_TICKS(2));
    transmissionState = radio.startTransmit(message);
#ifdef DEBUG
    Serial.print("[LORA]: ");
    Serial.println(message);
#endif
  } else {
    Serial.print("STH WENT REALLY FUCKING WRONG");
  }
  while (!transmittedFlag) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
}  // namespace lora

#endif  // TBEAM