#include <EEPROM.h>

constexpr std::size_t MEMORY_SIZE = 512;

namespace memory {

struct Config {
    int intValue;
    float floatValue;
    char stringValue[50];
};

inline Config config;

void init();
void save_config(Config &);
void load_config(Config &);
void print_data();

} // namespace memory
