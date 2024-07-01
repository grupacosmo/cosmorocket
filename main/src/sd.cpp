#include "sd.h"
namespace sd {

void init() {
    if (!SD.begin(sd::DEFAULT_DATA_PIN)) {
        Serial.println("Card Mount Failed");
        return;
    }

    if (SD.cardType() == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.println("SD card initialized.");
}

void write(const char *data) {
    if (SD.exists("/data.txt"))
        File = SD.open("/data.txt", FILE_APPEND);
    else
        File = SD.open("/data.txt", FILE_WRITE);
    if (!File) {
        Serial.println("Failed to open file for writing");
        return;
    }
    File.println(data);
    File.close();
}

void write(const String &data) { write(data.c_str()); }

void read() {
    File = SD.open("/data.txt", FILE_READ);
    if (!File) {
        Serial.println("Failed to open file for reading");
        return;
    }
    if (!File.available()) {
        Serial.println("File empty");
    }
    while (File.available()) {
        Serial.write(File.read());
    }
    File.close();
}
} // namespace sd