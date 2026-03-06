#include "common.h"

#include <expected>

#include "esp_log.h"
#include "freertos/idf_additions.h"

constexpr inline const char *TAG = "COMMON";

SemaphoreHandle_t g_main_semaphore = nullptr;

auto mainSemaphoreInit() -> std::expected<Success, Error> {
    g_main_semaphore = xSemaphoreCreateBinary();
    if (g_main_semaphore == nullptr) {
        ESP_LOGE(TAG, "Main semaphore init failed");
        return std::unexpected(Error::MAIN_SEMAPHORE_INIT_FAILED);
    }
    mainSemaphoreGive();
    return Success{};
}

void mainSemaphoreGive() { xSemaphoreGive(g_main_semaphore); }

void mainSemaphoreTake() {
    while (xSemaphoreTake(g_main_semaphore, 500) != pdTRUE) {
    }
}
