#ifndef lib_datagram_h
#define lib_datagram_h
#include <Arduino.h>
#include <rotary_encoder_config.h>

void decodeDatagram(uint8_t* buffer, uint16_t wordSize) {
    Serial.printf("Decoded Datagram: ");
    uint8_t crc = buffer[0];
    uint8_t header = buffer[1];
    uint16_t position1 = (buffer[2] << 8) + buffer[3];
    int16_t speed10 = (buffer[4] << 8) + buffer[5];
    int16_t speed11 = (buffer[6] << 8) + buffer[7];
    int16_t speed12 = (buffer[8] << 8) + buffer[9];
    int16_t speed13 = (buffer[10] << 8) + buffer[11];
    int16_t speed14 = (buffer[12] << 8) + buffer[13];

    size_t offset = 4 + 2 * SPEEDS_COUNT_TO_KEEP;
    uint16_t position2 = (buffer[offset] << 8) + buffer[offset + 1];
    int16_t speed20 = (buffer[offset + 2] << 8) + buffer[offset + 3];
    int16_t speed21 = (buffer[offset + 4] << 8) + buffer[offset + 5];
    int16_t speed22 = (buffer[offset + 6] << 8) + buffer[offset + 7];
    int16_t speed23 = (buffer[offset + 8] << 8) + buffer[offset + 9];
    int16_t speed24 = (buffer[offset + 10] << 8) + buffer[offset + 11];

    Serial.printf("CRC: %d ; Header: %d ; Position1: %d ; Speeds1: [%d, %d, %d, %d, %d, ...]; Position2: %d ; Speeds2: [%d, %d, %d, %d, %d, ...]\n", crc, header, position1, speed10, speed11, speed12, speed13, speed14, position2, speed20, speed21, speed22, speed23, speed24);
}

#endif