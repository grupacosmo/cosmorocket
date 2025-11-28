#include "nmea_decoder.h"

#include <cstdint>
#include <cstring>
#include <iostream>

namespace nmea_decoder {

constexpr inline size_t SENTENCE_PARTS = 15;

static void minutesToString(char *&outPtr, int frac) {
    *outPtr++ = frac / 1'000'000 + '0';
    *outPtr++ = (frac / 100'000) % 10 + '0';
    *outPtr++ = (frac / 10'000) % 10 + '0';
    *outPtr++ = (frac / 1'000) % 10 + '0';
    *outPtr++ = (frac / 100) % 10 + '0';
    *outPtr++ = (frac / 10) % 10 + '0';
    *outPtr++ = frac % 10 + '0';
}

// Example: $GNGGA,110348.00,2503.51538,N,03672.57466,E,1,06,2.81,240.1,M,39.9,M,,*44

Result decode(std::array<char, MAX_OUTPUT_SIZE> &out,
              const std::array<char, MAX_SENTENCE_SIZE> &nmea_sentence) {
    const char *ptr = nmea_sentence.data();

    const char *part[SENTENCE_PARTS];
    int count = 0;
    part[count++] = ptr;
    while (*ptr && count < SENTENCE_PARTS) {
        if (*ptr == ',') {
            part[count++] = ptr + 1;
        }
        ptr++;
    }
    if (count < SENTENCE_PARTS)  // Invalid data
        return WRONG_DATA_FORMAT;

    int header_index = 0;
    char header[16];
    for (const char *pos = part[0]; header_index < 16 && pos < part[1] - 1; pos++)
        header[header_index++] = *pos;
    if (header_index >= 16) return WRONG_DATA_FORMAT;
    header[header_index] = 0;

    if (std::strcmp(header, "$GNGGA") != 0) return WRONG_HEADER;

    char *outPtr = out.data();

    // Time format: HHMMSS.ss -> HH:MM:SS.ss
    if (part[1][0] != ',') {
        *outPtr++ = part[1][0];
        *outPtr++ = part[1][1];
        *outPtr++ = ':';
        *outPtr++ = part[1][2];
        *outPtr++ = part[1][3];
        *outPtr++ = ':';
        *outPtr++ = part[1][4];
        *outPtr++ = part[1][5];
        *outPtr++ = '.';
        *outPtr++ = part[1][7];
        *outPtr++ = part[1][8];
    }

    *outPtr++ = ',';

    int frac;

    if (part[2][0] != ',') {
        // Latitude format: DDMM.mmmmm -> DD.ddddddd
        frac = ((part[2][2] - '0') * 100'000'000LL + (part[2][3] - '0') * 10'000'000LL +
                (part[2][5] - '0') * 1'000'000LL + (part[2][6] - '0') * 100'000LL +
                (part[2][7] - '0') * 10'000LL + (part[2][8] - '0') * 1'000LL +
                (part[2][9] - '0') * 100LL) /
               60;
        if (part[2][0] != '0') *outPtr++ = part[2][0];
        *outPtr++ = part[2][1];
        *outPtr++ = '.';
        minutesToString(outPtr, frac);
        *outPtr++ = part[3][0];
    }

    *outPtr++ = ',';

    if (part[4][0] != ',') {
        // Longitude format: DDDMM.mmmmm -> DDD.ddddddd
        frac = ((part[4][3] - '0') * 100'000'000LL + (part[4][4] - '0') * 10'000'000LL +
                (part[4][6] - '0') * 1'000'000LL + (part[4][7] - '0') * 100'000LL +
                (part[4][8] - '0') * 10'000LL + (part[4][9] - '0') * 1'000LL +
                (part[4][10] - '0') * 100LL) /
               60;
        if (part[4][0] != '0') *outPtr++ = part[4][0];
        if (part[4][0] != '0' || part[4][1] != '0') *outPtr++ = part[4][1];
        *outPtr++ = part[4][2];
        *outPtr++ = '.';
        minutesToString(outPtr, frac);
        *outPtr++ = part[5][0];
    }

    *outPtr++ = ',';

    char *outEndPtr = out.data() + MAX_OUTPUT_SIZE - 1;

    // Read altitude
    for (const char *pos = part[9]; outPtr < outEndPtr && pos < part[10] - 1; pos++)
        *outPtr++ = *pos;
    if (outPtr >= outEndPtr) return BUFFER_OVERFLOW;

    *outPtr++ = ',';

    for (const char *pos = part[8]; outPtr < outEndPtr && pos < part[9] - 1; pos++)
        *outPtr++ = *pos;
    if (outPtr >= outEndPtr) return BUFFER_OVERFLOW;

    *outPtr++ = '\0';

    return SUCCESS;
}

}  // namespace nmea_decoder
