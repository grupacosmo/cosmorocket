#ifdef TBEAM

#define SD_CUSTOM_SPI
#define SD_SCK 25
#define SD_MISO 15
#define SD_MOSI 32
#define SD_SS 33

#define SDA_PIN_ 21
#define SCL_PIN_ 22

#elif defined(LOLIN)

#define SDA_PIN_ 13
#define SCL_PIN_ 15

#endif
