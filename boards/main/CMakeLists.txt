file(GLOB_RECURSE MAIN_SOURCES "*.cpp")

idf_component_register(SRCS ${MAIN_SOURCES}
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_timer spi_flash BME280 LSM6DSO ring_buffer minmea esp-idf-sx126x-minimal triplebuffer-sync)
