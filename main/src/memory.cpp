#include "memory.h"

namespace memory {

// Private
namespace {

constexpr std::uint8_t CONFIG_ADDR = 0;
constexpr std::size_t MEMORY_SIZE = 512;

Config config;

} // namespace

void init() {
    EEPROM.begin(MEMORY_SIZE);
    load_config(config);
}

void save_config() {
    EEPROM.put(CONFIG_ADDR, config);
    EEPROM.commit();
}

void load_config() {
    EEPROM.get(CONFIG_ADDR, config);
}

void print_debug() {
    Serial.print("Read int: ");
    Serial.println(config.intValue);
    Serial.print("Read float: ");
    Serial.println(config.floatValue);
    Serial.print("Read string: ");
    Serial.println(config.stringValue);
}

} // namespace memory