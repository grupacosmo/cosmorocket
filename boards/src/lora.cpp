#include "lora.h"
namespace lora {
Radio radio = new RadioModule();
int transmissionState = RADIOLIB_ERR_NONE;
volatile bool transmittedFlag = false;
int count = 0;

void setFlag(void) { transmittedFlag = true; }

void init() {
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
}
void lora_task(void* pvParameters) {
  for (;;) {
    if (transmittedFlag) {
      transmittedFlag = false;

      if (transmissionState == RADIOLIB_ERR_NONE) {
        Serial.println("transmission finished!");

      } else {
        Serial.print("failed, code ");
        Serial.println(transmissionState);
      }
      radio.finishTransmit();
      vTaskDelay(pdMS_TO_TICKS(100));
      Serial.print("[SX1278] Sending another packet ... ");
      String str = "Hello World! #" + String(count++);
      transmissionState = radio.startTransmit(str);
    }
  }
}
}  // namespace lora