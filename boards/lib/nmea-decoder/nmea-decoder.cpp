#pragma once

namespace nmea_decoder {
void decode(const char *nmea_sentence, char *outPtr) {
    const char *ptr = nmea_sentence;
    const char *part[12];
    int count = 0;
    char output[32] = {};  // buffer
    char *outPtr = output;

    part[count++] = ptr;
    while (*ptr && count < 12) {
        if (*ptr == ',') {
            part[count++] = ptr + 1;
        }
        ptr++;
    }

    // Time format: HHMMSS.sss -> HH:MM:SS
    *outPtr++ = part[1][0];
    *outPtr++ = part[1][1];
    *outPtr++ = ':';
    *outPtr++ = part[1][2];
    *outPtr++ = part[1][3];
    *outPtr++ = ':';
    *outPtr++ = part[1][4];
    *outPtr++ = part[1][5];
    *outPtr++ = '	';

    // Latitude format: DDMM.mmmm,N/S -> DD.dddddN/S
    if (part[3][0] != '0') *outPtr++ = part[3][0];
    *outPtr++ = part[3][1];
    int frac = (((part[3][2] - '0') * 10 + (part[3][3] - '0')) * 10000 + (part[3][5] - '0') * 1000 +
                (part[3][6] - '0') * 100 + (part[3][7] - '0') * 10) /
               60;
    *outPtr++ = '.';
    *outPtr++ = frac / 1000 + '0';
    *outPtr++ = frac / 100 % 10 + '0';
    *outPtr++ = frac / 10 % 10 + '0';
    *outPtr++ = frac % 10 + '0';
    *outPtr++ = part[4][0];
    *outPtr++ = '	';

    // Longitude format: DDDMM.mmmm,E/W -> ±DDD.dddddE/W
    if (part[6][0] == 'W') *outPtr++ = '-';
    if (part[5][0] != '0') *outPtr++ = part[5][0];
    if (part[5][1] != '0') *outPtr++ = part[5][1];
    *outPtr++ = part[5][2];
    frac = (((part[5][3] - '0') * 10 + (part[5][4] - '0')) * 10000 + (part[5][6] - '0') * 1000 +
            (part[5][7] - '0') * 100 + (part[5][8] - '0') * 10) /
           60;
    *outPtr++ = '.';
    *outPtr++ = frac / 1000 + '0';
    *outPtr++ = frac / 100 % 10 + '0';
    *outPtr++ = frac / 10 % 10 + '0';
    *outPtr++ = frac % 10 + '0';
    *outPtr++ = part[6][0];
    *outPtr = '\0';
}
}  // namespace nmea_decoder
