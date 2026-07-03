#include "lora.h"

#include <cstdint>
#include <cstdio>
#include <format>

extern "C" {
#include <sx126x.h>
}

#include <TripleBuffer.hxx>
#include <array>
#include <expected>

#include "common.h"
#include "esp_log.h"
#include "sdkconfig.h"

namespace lora {

struct CurrentData {
    TripleBuffer<bme280::Data> barometer_data;
    TripleBuffer<std::array<int16_t, 3>> acceleration_data;
    TripleBuffer<std::array<int16_t, 3>> angular_rate_data;
    TripleBuffer<gps::Data> gps_data;
} g_latest_data;

constexpr inline const char *TAG = "LORA";

constexpr inline size_t LORA_TASK_STACK_SIZE = 8192;

static void task_tx(void *pvParameters) {
    while (true) {
        ESP_LOGI(TAG, "Sending data...");

        std::array<char, 255> data{};
        auto length = std::format_to_n(
                          data.begin(), data.size(), "{}",
                          g_latest_data.barometer_data.readLast().air_pressure)
                          .size;

        // Wait for transmission to complete
        if (not LoRaSend(reinterpret_cast<uint8_t *>(data.data()),
                         static_cast<int16_t>(length), SX126x_TXMODE_SYNC)) {
            ESP_LOGE(TAG, "Could not send data");
        }

        if (int lost = GetPacketLost(); lost != 0) {
            ESP_LOGW(TAG, "%d packets have been lost", lost);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

auto init() -> std::expected<Success, Error> {
    lora_gpio_t pin_config = {
        .CONFIG_MISO_GPIO = CONFIG_LORA_MISO_GPIO,
        .CONFIG_MOSI_GPIO = CONFIG_LORA_MOSI_GPIO,
        .CONFIG_SCLK_GPIO = CONFIG_LORA_SCK_GPIO,  // xsiao
        .CONFIG_NSS_GPIO = CONFIG_LORA_NSS_GPIO,
        .CONFIG_RST_GPIO = CONFIG_LORA_NRST_GPIO,
        .CONFIG_BUSY_GPIO = CONFIG_LORA_BUSY_GPIO,

        .CONFIG_TXEN_GPIO = -1,
        .CONFIG_RXEN_GPIO = -1};
    LoRaInit(&pin_config);

    lora_begin_params_t lora = {.frequencyInHz = 866000000,
                                .txPowerInDbm = 10,
                                .tcxoVoltage = 0,
                                .useRegulatorLDO = false};
    lora.tcxoVoltage = 3.1f;      // use TCXO
    lora.useRegulatorLDO = true;  // use DCDC + LDO
    if (LoRaBegin(&lora) != 0) {
        return std::unexpected(Error::LORA_INIT_FAILED);
    }

    lora_config_params_t lora_config = {.spreadingFactor = 7,
                                        .bandwidth = 4,
                                        .codingRate = 1,
                                        .preambleLength = 8,
                                        .payloadLen = 0,
                                        .crcOn = true,
                                        .invertIrq = false};
    LoRaConfig(&lora_config);

    if (xTaskCreate(&task_tx, "TRANSMIT_TASK", LORA_TASK_STACK_SIZE, nullptr,
                    /* uxPriority = */ 8, nullptr) != pdPASS) {
        return std::unexpected(Error::LORA_INIT_FAILED);
    }

    return Success{};
}

void postBarometerData(const bme280::Data &data) {
    auto &buffer = g_latest_data.barometer_data;
    buffer.write(data);
    buffer.flipWriter();
}

void postAccelerationData(const std::array<int16_t, 3> &data) {
    auto &buffer = g_latest_data.acceleration_data;
    buffer.write(data);
    buffer.flipWriter();
}

void postAngularRateData(const std::array<int16_t, 3> &data) {
    auto &buffer = g_latest_data.angular_rate_data;
    buffer.write(data);
    buffer.flipWriter();
}

void postGpsData(const gps::Data &data) {
    auto &buffer = g_latest_data.gps_data;
    buffer.write(data);
    buffer.flipWriter();
}

}  // namespace lora
