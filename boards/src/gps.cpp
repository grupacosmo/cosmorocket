#include "gps.h"

#include <HardwareSerial.h>
#include <driver/uart.h>

#include "board_config.h"

namespace gps {

constexpr inline uint8_t UART_BUS_NUMBER = UART_NUM_1;  // 0 is Serial
constexpr inline TickType_t UART_TIMEOUT = 100;
constexpr inline size_t UART_BUFFER_SIZE = 1024;
constexpr inline size_t UART_QUEUE_SIZE = 20;
constexpr inline size_t UART_PATTERN_QUEUE_SIZE = 20;
constexpr inline size_t LINE_BUFFER_SIZE = 128;

QueueHandle_t g_uart_queue;
char g_line_buffer[LINE_BUFFER_SIZE];

static void readLine() {
    // Get posision of detected '\n' character
    int pos = uart_pattern_pop_pos(UART_BUS_NUMBER);
    if (pos == -1) {
        Serial.println("GPS: UART pattern queue overflow");
        return;
    }
    int to_read = pos + 1;  // Data length plus '\n'
    if (to_read >= LINE_BUFFER_SIZE) {
        Serial.println("GPS: Too much data in one NMEA line");
        uart_flush_input(UART_BUS_NUMBER);
        xQueueReset(g_uart_queue);
        return;
    }

    int read_size = uart_read_bytes(UART_BUS_NUMBER, g_line_buffer, to_read, UART_TIMEOUT);
    if (read_size != to_read) Serial.println("GPS: UART read error");
    g_line_buffer[read_size - 1] = '\0';  // Replace '\n' with end of string
    Serial.printf("GPS data: %s\n", g_line_buffer);
}

static void uartEventTask(void *pvParameters) {
    uart_event_t event;

    while (true) {
        // Block until an event is received
        if (xQueueReceive(g_uart_queue, reinterpret_cast<void *>(&event), portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    // We do not read data here, only in UART_PATTERN_DET
                    break;

                case UART_PATTERN_DET:
                    readLine();
                    break;

                case UART_FIFO_OVF:
                    Serial.println("GPS: UART FIFO OVERFLOW");
                    uart_flush_input(UART_BUS_NUMBER);
                    xQueueReset(g_uart_queue);
                    break;

                case UART_BUFFER_FULL:
                    Serial.println("GPS: UART BUFFER FULL");
                    uart_flush_input(UART_BUS_NUMBER);
                    xQueueReset(g_uart_queue);
                    break;

                case UART_BREAK:
                    // Serial.println("UART BREAK");
                    break;

                case UART_PARITY_ERR:
                    Serial.println("GPS: UART PARITY ERR");
                    break;

                case UART_FRAME_ERR:
                    Serial.println("GPS: UART FRAME ERR");
                    break;

                default:
                    Serial.println("GPS: UART UNKNOWN EVENT");
                    break;
            }
        }
    }
}

void init() {
    uart_config_t uart_config = {
        .baud_rate = board_config::GPS_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 112,  // Not used but must be defined. Default HardwareSerial value.
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_BUS_NUMBER, UART_BUFFER_SIZE, 0, UART_QUEUE_SIZE, &g_uart_queue, 0);
    uart_param_config(UART_BUS_NUMBER, &uart_config);

    uart_set_pin(UART_BUS_NUMBER, board_config::GPS_UART_TX_PIN, board_config::GPS_UART_RX_PIN,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Setup pattern recognition to detect '\n' as the and of NMEA line
    uart_enable_pattern_det_baud_intr(UART_BUS_NUMBER, '\n', 1, 9, 0, 0);
    uart_pattern_queue_reset(UART_BUS_NUMBER, UART_PATTERN_QUEUE_SIZE);

    xTaskCreate(uartEventTask, "gpsUartEventTask", /* ucStackDepth = */ 2048, NULL,
                /* uxPriority = */ 12, NULL);
}

}  // namespace gps