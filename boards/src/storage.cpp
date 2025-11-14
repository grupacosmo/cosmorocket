#include "storage.h"

#include <HardwareSerial.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <ring_buffer.hpp>

namespace storage {

// 200ms delay between data flushes
static constexpr inline int DATA_FLUSH_INTERVAL = 1000;

SemaphoreHandle_t g_semaphore;
esp_timer_handle_t g_timer;

// Thread safe (atomic) Single Procuder Single Consumer ring buffer
SPSCQueue<barometer::Data, 32> g_barometer_data_buffer;
SPSCQueue<accelerometer::Acceleration, 1024> g_acceleration_buffer;
SPSCQueue<accelerometer::AngularRate, 1024> g_angular_rate_buffer;
SPSCQueue<gps::Data, 64> g_gps_data_buffer;

static void timerEvent(void *arg) { xSemaphoreGive(g_semaphore); }

static void flushTask(void *pvParameters) {
    while (true) {
        if (xSemaphoreTake(g_semaphore, 500) == pdTRUE) {
            Serial.println("Storage flush started");

            Serial.println("BME280 data:");
            while (!g_barometer_data_buffer.empty()) {
                barometer::Data data{};
                g_barometer_data_buffer.pop(data);

                Serial.printf("%6.2fPa, %2.2f%%, %2.2fC\n", data.air_pressure, data.humidity,
                              data.temperature);
            }

            Serial.println("LSM6DSO32 acceleration:");
            int cnt = 0;
            while (!g_acceleration_buffer.empty()) {
                accelerometer::Acceleration data{};
                g_acceleration_buffer.pop(data);

                if (cnt < 5) Serial.printf("X: %6d Y: %6d Z: %6d\n", data[0], data[1], data[2]);

                cnt++;
            }
            Serial.printf("And %d more records.\n", cnt - 5);

            Serial.println("LSM6DSO32 angular rate:");
            cnt = 0;
            while (!g_angular_rate_buffer.empty()) {
                accelerometer::AngularRate data{};
                g_angular_rate_buffer.pop(data);

                if (cnt < 5) Serial.printf("X: %6d Y: %6d Z: %6d\n", data[0], data[1], data[2]);

                cnt++;
            }
            Serial.printf("And %d more records.\n", cnt - 5);

            Serial.println("GPS data:");
            while (!g_gps_data_buffer.empty()) {
                gps::Data data;
                g_gps_data_buffer.pop(data);

                Serial.printf("%s\n", data.data());
            }

            Serial.println("Storage flush complete.");
        }
    }
}

void init() {
    const esp_timer_create_args_t timer_config = {.callback = timerEvent,
                                                  .arg = nullptr,
                                                  .dispatch_method = ESP_TIMER_TASK,
                                                  .name = "storage-flush-timer",
                                                  .skip_unhandled_events = true};
    esp_timer_create(&timer_config, &g_timer);
    esp_timer_start_periodic(g_timer, DATA_FLUSH_INTERVAL * 1000);

    g_semaphore = xSemaphoreCreateBinary();

    xTaskCreate(flushTask, "storageFlushTask", /* ucStackDepth = */ 4096, NULL,
                /* uxPriority = */ 11, NULL);
}

void postBarometerData(const barometer::Data &data) {
    if (!g_barometer_data_buffer.push(data))
        Serial.println("Storage barometer data buffer overflow");
}

void postAccelerationData(const accelerometer::Acceleration &data) {
    if (!g_acceleration_buffer.push(data)) Serial.println("Storage acceleration buffer overflow");
}

void postAngularRateData(const accelerometer::AngularRate &data) {
    if (!g_angular_rate_buffer.push(data)) Serial.println("Storage angular rate buffer overflow");
}

void postGpsData(const gps::Data &data) {
    if (!g_gps_data_buffer.push(data)) Serial.println("GPS data buffer overflow");
}

}  // namespace storage
