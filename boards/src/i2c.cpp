#include "i2c.h"

#include <HardwareSerial.h>
#include <esp32-hal-i2c.h>

#include <cstring>

#include "board_config.h"

namespace i2c {

constexpr inline uint8_t BUS_NUMBER = 0;
constexpr inline uint32_t TIMEOUT = 50;

uint8_t g_buffer[0x100];

void init() {
    i2cInit(BUS_NUMBER, board_config::I2C_SLAVE_SCL_PIN, board_config::I2C_SLAVE_SDA_PIN, 400000);
}

bool read(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size) {
    // Serial.printf("I2C: Requesting %d bytes from reg 0x%02X\n", size, reg);
    size_t read_count = 0;
    if (i2cWriteReadNonStop(BUS_NUMBER, addr, &reg, 1, buffer, size, TIMEOUT, &read_count)) {
        Serial.println("I2C: Failed to read");
        return false;
    }
    if (read_count != size) {
        Serial.println("I2C: Read size mismatch");
        return false;
    }
    return true;
}

bool write(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size) {
    // Serial.printf("I2C: Sending %d bytes to reg 0x%02X\n", size, reg);
    g_buffer[0] = reg;
    memcpy(&g_buffer[1], buffer, size);
    if (i2cWrite(BUS_NUMBER, addr, g_buffer, size + 1, TIMEOUT)) {
        Serial.println("I2C: Failed to write");
        return false;
    }
    return true;
}

}  // namespace i2c
