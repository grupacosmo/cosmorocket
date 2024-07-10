#include "sd_card.h"

// TODO make it better
#ifndef SD_SCK
// t-bean
#define SD_SCK 25 
#define SD_MISO 15 
#define SD_MOSI 32 
#define SD_SS 33
#endif

namespace sd {

namespace {
fs::File File;
}
void init() {
    // TODO if lolin then SD.begin(sd::DEFAULT_DATA_PIN) // without spi
    SPIClass spi(VSPI);
    spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS);

    if (!SD.begin(sd::DEFAULT_DATA_PIN, spi)) {
        Serial.println("Card Mount Failed");
        return;
    }

    if (SD.cardType() == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.println("SD card initialized.");
}

void write(const char *data, const char *filename = "/data.csv") {
    if (SD.exists(filename))
        File = SD.open(filename, FILE_APPEND);
    else
        File = SD.open(filename, FILE_WRITE);
    if (!File) {
        Serial.println("Failed to open file for writing");
        return;
    }
    File.println(data);
    File.close();
}

void write(const String &data, const String &filename = "/data.csv") {
    write(data.c_str(), filename.c_str());
}

void read(const char *filename = "/data.csv") {
    File = SD.open(filename, FILE_READ);
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
void read(const String &filename = "/data.csv") { read(filename.c_str()); }
} // namespace sd