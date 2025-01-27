#include "display.h"
#include "bmp.h"
#include "gps.h"
#include "mpu.h"
#include <SPI.h>
#include <Adafruit_GFX.h>

namespace display {

// Private
namespace {

#define OLED_DC     4
#define OLED_CS     5

}

  SPIClass vspi(VSPI);
  Adafruit_SSD1306 disp(128, 64, &vspi, OLED_DC, -1, OLED_CS);

  int num;

  void init() {
    num = 0;
    if(!disp.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println(F("SSD1306 allocation failed"));
    }

    disp.drawPixel(10, 10, SSD1306_WHITE);

    disp.display();
  }

  void display_task([[maybe_unused]] void* pvParameters)
  {
    for (;;)
    {
      disp.clearDisplay();
      disp.setTextSize(1);
      disp.setTextColor(WHITE);
      disp.setCursor(0, 0);
      bmp::print_debug();
      gps::print_debug();
      mpu::print_debug();
      disp.display();

      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

  Adafruit_SSD1306 *display_get()
  {
    return &disp;
  }
}
