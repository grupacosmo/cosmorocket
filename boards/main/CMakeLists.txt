file(GLOB_RECURSE MAIN_SOURCES "*.cpp")

idf_component_register(SRCS ${MAIN_SOURCES}
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_timer BME280 LSM6DSO ring_buffer)
