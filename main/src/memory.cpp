#include "memory.h"

#define CONFIG_ADDR 0

namespace memory {

void init() {
    EEPROM.begin(MEMORY_SIZE);
    load_config(config);
}

void save_config(Config &data) {
    EEPROM.put(CONFIG_ADDR, data);
    EEPROM.commit();
}
void load_config(Config &data) { EEPROM.get(CONFIG_ADDR, data); }

void print_data() {
    Serial.print("Read int: ");
    Serial.println(config.intValue);
    Serial.print("Read float: ");
    Serial.println(config.floatValue);
    Serial.print("Read string: ");
    Serial.println(config.stringValue);
}
} // namespace memory