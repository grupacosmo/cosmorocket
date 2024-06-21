#include <HardwareSerial.h>
#define GPS_SERIAL_NUM 1
#define GPS_RX_PIN 34
#define GPS_TX_PIN 12

namespace gps {

void init();
void gps_loop(void* pvParameters);

}  // namespace gps
