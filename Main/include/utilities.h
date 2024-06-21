
#pragma once
#define LILYGO_TBeam_V1_X

#ifndef LoRa_frequency
#define LoRa_frequency 433.0
#endif

#define UNUSE_PIN (0)

#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#define BUTTON_PIN 38
#define BUTTON_PIN_MASK GPIO_SEL_38
#define I2C_SDA 21
#define I2C_SCL 22
#define PMU_IRQ 35

#define RADIO_SCLK_PIN 5
#define RADIO_MISO_PIN 19
#define RADIO_MOSI_PIN 27
#define RADIO_CS_PIN 18
#define RADIO_DIO0_PIN 26
#define RADIO_RST_PIN 23
#define RADIO_DIO1_PIN 33
#define RADIO_BUSY_PIN 32

#define BOARD_LED 4
#define LED_ON LOW
#define LED_OFF HIGH

#define GPS_BAUD_RATE 9600
#define HAS_GPS
