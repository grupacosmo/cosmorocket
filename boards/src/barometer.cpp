#include "barometer.h"

#include <HardwareSerial.h>

#include "bmp280/interface/driver_bmp280_interface.h"
#include "bmp280/src/driver_bmp280.h"

namespace barometer {

bmp280_handle_t g_sensor;
uint32_t raw_temperature;
uint32_t raw_pressure;

void init() {
    DRIVER_BMP280_LINK_INIT(&g_sensor, bmp280_handle_t);
    DRIVER_BMP280_LINK_IIC_INIT(&g_sensor, bmp280_interface_iic_init);
    DRIVER_BMP280_LINK_IIC_DEINIT(&g_sensor, bmp280_interface_iic_deinit);
    DRIVER_BMP280_LINK_IIC_READ(&g_sensor, bmp280_interface_iic_read);
    DRIVER_BMP280_LINK_IIC_WRITE(&g_sensor, bmp280_interface_iic_write);
    DRIVER_BMP280_LINK_SPI_INIT(&g_sensor, bmp280_interface_spi_init);
    DRIVER_BMP280_LINK_SPI_DEINIT(&g_sensor, bmp280_interface_spi_deinit);
    DRIVER_BMP280_LINK_SPI_READ(&g_sensor, bmp280_interface_spi_read);
    DRIVER_BMP280_LINK_SPI_WRITE(&g_sensor, bmp280_interface_spi_write);
    DRIVER_BMP280_LINK_DELAY_MS(&g_sensor, bmp280_interface_delay_ms);
    DRIVER_BMP280_LINK_DEBUG_PRINT(&g_sensor, bmp280_interface_debug_print);

    if (bmp280_set_interface(&g_sensor, BMP280_INTERFACE_IIC) != 0) {
        Serial.println("bmp280: set interface failed.\n");
        return;
    }

    if (bmp280_set_addr_pin(&g_sensor, BMP280_ADDRESS_ADO_LOW) != 0) {
        Serial.println("bmp280: set addr pin failed.\n");
        return;
    }

    if (bmp280_init(&g_sensor) != 0) {
        Serial.println("bmp280: init failed.\n");
        return;
    }

    if (bmp280_set_temperatue_oversampling(&g_sensor, BMP280_OVERSAMPLING_x1) != 0) {
        Serial.println("bmp280: set temperatue oversampling failed.\n");
        (void)bmp280_deinit(&g_sensor);
        return;
    }

    if (bmp280_set_pressure_oversampling(&g_sensor, BMP280_OVERSAMPLING_x16) != 0) {
        Serial.println("bmp280: set pressure oversampling failed.\n");
        (void)bmp280_deinit(&g_sensor);
        return;
    }

    if (bmp280_set_standby_time(&g_sensor, BMP280_STANDBY_TIME_0P5_MS) != 0) {
        Serial.println("bmp280: set standby time failed.\n");
        (void)bmp280_deinit(&g_sensor);
        return;
    }

    if (bmp280_set_filter(&g_sensor, BMP280_FILTER_COEFF_16) != 0) {
        Serial.println("bmp280: set filter failed.\n");
        (void)bmp280_deinit(&g_sensor);
        return;
    }

    if (bmp280_set_mode(&g_sensor, BMP280_MODE_NORMAL) != 0) {
        Serial.println("bmp280: set mode failed.\n");
        (void)bmp280_deinit(&g_sensor);
        return;
    }
}

void getData(Data::Pressure &pressure, Data::Temperature &temperature) {
    if (bmp280_read_temperature_pressure(&g_sensor, &raw_temperature, &temperature, &raw_pressure,
                                         &pressure) != 0) {
        (void)bmp280_deinit(&g_sensor);  // Return value ignored
        Serial.println("bmp280: read failed. Reinitialising...");
        init();
        temperature = 0.0f;
        pressure = 0.0f;
    }
}

}  // namespace barometer
