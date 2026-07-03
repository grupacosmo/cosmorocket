#include "gps.h"

#include <driver/uart.h>
#include <minmea/minmea.h>

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstring>
#include <format>

#include "esp_log.h"
#include "esp_timer.h"
#include "lora.h"
#include "storage.h"

namespace gps {

constexpr inline const char *TAG = "GPS";

constexpr inline uart_port_t UART_BUS_NUMBER = UART_NUM_1;
constexpr inline TickType_t UART_TIMEOUT = 100;  // 100 ticks

// Size of the FIFO buffer allocated by the driver
constexpr inline size_t UART_BUFFER_SIZE = 1024;

constexpr inline size_t UART_QUEUE_SIZE = 20;
constexpr inline size_t UART_PATTERN_QUEUE_SIZE = 20;

constexpr inline int MAX_NMEA_SENTENCE_SIZE = 196;

constexpr inline size_t GPS_UART_TASK_STACK_SIZE = 8192;

// Event queue operated by the UART driver
QueueHandle_t g_uart_queue;

// Temporary buffer to store raw NMEA data until it's decoded.
std::array<char, MAX_NMEA_SENTENCE_SIZE> g_line_buffer;

int i = 0;

template <size_t N>
static void decodeMinmeaFloat(std::array<char, N> &dst, minmea_float value) {}

static void readLine() {
    // Get position of detected '\n' character
    int pos = uart_pattern_pop_pos(UART_BUS_NUMBER);
    if (pos == -1) {
        ESP_LOGW(TAG, "UART pattern queue overflow");
        return;
    }
    int to_read = pos + 1;  // Data length plus '\n'
    if (to_read >= MAX_NMEA_SENTENCE_SIZE) {
        ESP_LOGW(TAG, "Too much data in one NMEA line");
        uart_flush_input(UART_BUS_NUMBER);
        xQueueReset(g_uart_queue);
        return;
    }

    int read_size = uart_read_bytes(UART_BUS_NUMBER, g_line_buffer.data(),
                                    to_read, UART_TIMEOUT);
    if (read_size != to_read) ESP_LOGW(TAG, "UART read error");
    g_line_buffer[read_size - 1] = '\0';  // Replace '\n' with end of string

    // char *data = g_line_buffer.data();
    // ESP_LOGI(TAG, "%s", data);

    // Our GPS sends: $GPGSV $GBGSV $GAGSV $GQGSV $GNGSA $GNVTG $GNGGA $GNRMC

    switch (minmea_sentence_id(g_line_buffer.data(), false)) {
        case MINMEA_SENTENCE_GGA: {  // sentence of type GGA contains altitude
            struct minmea_sentence_gga frame{};
            if (minmea_parse_gga(&frame, g_line_buffer.data())) {
                gps::Data data = {.timestamp = esp_timer_get_time(),
                                  .parsed_data = frame};
                storage::postGpsData(data);

                std::array<char, 128> text{};
                std::format_to_n(
                    text.begin(), text.size() - 1,
                    "{:02d}:{:02d}:{:02d}.{:d} {} {} alt: {}", frame.time.hours,
                    frame.time.minutes, frame.time.seconds,
                    frame.time.microseconds / 100000, frame.latitude.value,
                    frame.longitude.value, frame.altitude.value);
                // ESP_LOGI(TAG, "%s", text.data());
            }
            break;
        }
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame{};
            if (minmea_parse_rmc(&frame, g_line_buffer.data())) {
                gps::Data data = {.timestamp = esp_timer_get_time(),
                                  .parsed_data = frame};
                storage::postGpsData(data);
                lora::postGpsData(data);
            }
            break;
        }
        case MINMEA_SENTENCE_GLL: {
            struct minmea_sentence_gll frame{};
            if (minmea_parse_gll(&frame, g_line_buffer.data())) {
                gps::Data data = {.timestamp = esp_timer_get_time(),
                                  .parsed_data = frame};
                storage::postGpsData(data);
                lora::postGpsData(data);
            }
            break;
        }
        case MINMEA_SENTENCE_VTG: {
            struct minmea_sentence_vtg frame;
            if (minmea_parse_vtg(&frame, g_line_buffer.data())) {
                gps::Data data = {.timestamp = esp_timer_get_time(),
                                  .parsed_data = frame};
                storage::postGpsData(data);
                lora::postGpsData(data);
            }
            break;
        }
        default:
            break;
    }
}

static void uartEventTask(void *pvParameters) {
    uart_event_t event;

    while (true) {
        // UART driver sends an event to g_uart_queue when data is received.
        // Stops the task and waits until an event is received
        if (xQueueReceive(g_uart_queue, reinterpret_cast<void *>(&event),
                          portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    // We do not read data here, only in UART_PATTERN_DET
                    break;

                // Triggered by the driver when new line character is detected.
                // We can now decode the line.
                case UART_PATTERN_DET:
                    readLine();
                    break;

                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "UART FIFO OVERFLOW");
                    uart_flush_input(UART_BUS_NUMBER);
                    xQueueReset(g_uart_queue);
                    break;

                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "UART BUFFER FULL");
                    uart_flush_input(UART_BUS_NUMBER);
                    xQueueReset(g_uart_queue);
                    break;

                case UART_BREAK:
                    break;

                case UART_PARITY_ERR:
                    ESP_LOGW(TAG, "UART PARITY ERR");
                    break;

                case UART_FRAME_ERR:
                    ESP_LOGW(TAG, "UART FRAME ERR");
                    break;

                default:
                    ESP_LOGW(TAG, "UART UNKNOWN EVENT");
                    break;
            }
        }
    }
}

void init() {
    // Configure UART interface to read the data from GPS
    uart_config_t uart_config = {
        .baud_rate = CONFIG_GPS_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh =
            112,  // Not used but must be defined. Default HardwareSerial value.
        .source_clk = UART_SCLK_APB,  // Default clock source
        .flags = {}};

    uart_driver_install(UART_BUS_NUMBER, UART_BUFFER_SIZE, 0, UART_QUEUE_SIZE,
                        &g_uart_queue, 0);
    uart_param_config(UART_BUS_NUMBER, &uart_config);

    // TX pin is not used, but must be defined
    uart_set_pin(UART_BUS_NUMBER, CONFIG_GPS_UART_TX_GPIO,
                 CONFIG_GPS_UART_RX_GPIO, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    // Setup pattern recognition to detect '\n' as the end of NMEA line
    uart_enable_pattern_det_baud_intr(UART_BUS_NUMBER, '\n', 1, 9, 0, 0);
    uart_pattern_queue_reset(UART_BUS_NUMBER, UART_PATTERN_QUEUE_SIZE);

    xTaskCreate(uartEventTask, "gpsUartEventTask",
                /* ucStackDepth = */ GPS_UART_TASK_STACK_SIZE, NULL,
                /* uxPriority = */ 12, NULL);
}

}  // namespace gps