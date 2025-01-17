#include <Adafruit_SSD1306.h>

namespace display {
  void init();
  void display_task([[maybe_unused]] void* pvParameters);
  Adafruit_SSD1306 *display_get();
}