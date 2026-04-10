#include "i2c.h"

#include <cstring>
#include <expected>

#include "FreeRTOSConfig.h"
#include "common.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "portmacro.h"

namespace i2c {

constexpr inline const char *TAG = "I2C";

// The project uses only one I2C bus
constexpr inline i2c_port_t BUS_NUMBER = I2C_NUM_0;

constexpr inline int FREQUENCY = 400000;

i2c_master_bus_handle_t g_bus_handle;

auto init() -> std::expected<Success, Error> {
    i2c_master_bus_config_t conf{};
    conf.i2c_port = BUS_NUMBER;
    conf.sda_io_num = static_cast<gpio_num_t>(CONFIG_I2C_SDA_GPIO);
    conf.scl_io_num = static_cast<gpio_num_t>(CONFIG_I2C_SCL_GPIO);
    conf.clk_source = I2C_CLK_SRC_DEFAULT;
    conf.glitch_ignore_cnt = 7;
    conf.flags = {.enable_internal_pullup = 1, .allow_pd = 0};
    if (auto res = i2c_new_master_bus(&conf, &g_bus_handle); res != ESP_OK) {
        ESP_LOGE(TAG, "Config error (CODE: %d)", res);
        return std::unexpected(Error::I2C_INIT_FAILED);
    }

    return Success{};
}

auto getHandle() -> i2c_master_bus_handle_t { return g_bus_handle; }

}  // namespace i2c
