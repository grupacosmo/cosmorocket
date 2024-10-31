#pragma once
#include <EEPROM.h>

namespace memory {

struct Config {
    int intValue;
    float floatValue;
    char stringValue[50];
};

void init();
void save_config();
void load_config();
void print_debug();

} // namespace memory
