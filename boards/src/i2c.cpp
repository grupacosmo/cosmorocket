#include "i2c.h"

#include <HardwareSerial.h>
#include <esp32-hal-i2c.h>

#include <cstring>

#include "board_config.h"

namespace i2c {

// The project uses only one I2C bus
constexpr inline uint8_t BUS_NUMBER = 0;

// Bus timeout passed to ESP HAL API
constexpr inline uint32_t TIMEOUT = 50;

constexpr inline size_t MAX_SUPPORTED_TRANSFER_SIZE = 255;

// Temporary buffer for I2C write operations.
// The first byte is always the register address
uint8_t g_buffer[1 + MAX_SUPPORTED_TRANSFER_SIZE];

void init() { i2cInit(BUS_NUMBER, board_config::I2C_SDA_PIN, board_config::I2C_SCL_PIN, 400000); }

bool read(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t size) {
    if (size > MAX_SUPPORTED_TRANSFER_SIZE) {
        Serial.println("I2C: Error: unsupported read size");
        return false;
    }

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
    if (size > MAX_SUPPORTED_TRANSFER_SIZE) {
        Serial.println("I2C: Error: unsupported write size");
        return false;
    }

    g_buffer[0] = reg;
    memcpy(&g_buffer[1], buffer, size);
    if (i2cWrite(BUS_NUMBER, addr, g_buffer, 1 + size, TIMEOUT)) {
        Serial.println("I2C: Failed to write");
        return false;
    }
    return true;
}

}  // namespace i2c
