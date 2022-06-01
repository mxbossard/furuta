#ifndef lib_datagram_h
#define lib_datagram_h
#include <Arduino.h>
#include <rotary_encoder_config.h>
#include <lib_utils.h>

static const size_t COMMAND_PAYLOAD_SIZE = 5;
static const size_t DATA_PAYLOAD_SIZE = 48; 

void markCommandCrc8(uint8_t* buffer) {
    markCrc8(buffer, COMMAND_PAYLOAD_SIZE);
}

void markDataCrc8(uint8_t* buffer) {
    markCrc8(buffer, DATA_PAYLOAD_SIZE);
}

bool checkCommandCrc8(uint8_t* buffer) {
    return checkCrc8(buffer, COMMAND_PAYLOAD_SIZE);
}

bool checkDataCrc8(uint8_t* buffer) {
    return checkCrc8(buffer, DATA_PAYLOAD_SIZE);
}

void printCommandPayload(uint8_t* buffer) {
    Serial.printf("Decoded command payload: ");
    uint8_t crc8 = buffer[0];
    uint8_t length = buffer[1];
    uint8_t marker = buffer[2];
    uint8_t extraHeader = buffer[3];
    uint8_t command = buffer[4];

    Serial.printf("CRC8: %d ; Length: %d ; Marker: %d ; Header: %d ; Command: %d\n", crc8, length, marker, extraHeader, command);
}

void buildCommandPayload(uint8_t* buffer, uint8_t marker, uint8_t command) {
    buffer[1] = COMMAND_PAYLOAD_SIZE; // length
    buffer[2] = marker; // marker
    buffer[3] = 0; // extraHeader
    buffer[4] = command; // command
    markCommandCrc8(buffer);

    //printCommandPayload(buffer);
}

void printDataPayload(uint8_t* buffer, size_t speedsCount) {
    Serial.printf("Decoded data payload: ");
    uint8_t crc8 = buffer[0];
    uint8_t length = buffer[1];
    uint8_t marker = buffer[2];
    uint8_t extraHeader = buffer[3];
    uint16_t position1 = (buffer[4] << 8) + buffer[5];
    int16_t speed10 = (buffer[6] << 8) + buffer[7];
    int16_t speed11 = (buffer[8] << 8) + buffer[9];
    int16_t speed12 = (buffer[10] << 8) + buffer[11];
    int16_t speed13 = (buffer[12] << 8) + buffer[13];
    int16_t speed14 = (buffer[14] << 8) + buffer[15];

    size_t offset = 6 + 2 * speedsCount;
    uint16_t position2 = (buffer[offset] << 8) + buffer[offset + 1];
    int16_t speed20 = (buffer[offset + 2] << 8) + buffer[offset + 3];
    int16_t speed21 = (buffer[offset + 4] << 8) + buffer[offset + 5];
    int16_t speed22 = (buffer[offset + 6] << 8) + buffer[offset + 7];
    int16_t speed23 = (buffer[offset + 8] << 8) + buffer[offset + 9];
    int16_t speed24 = (buffer[offset + 10] << 8) + buffer[offset + 11];

    offset = 8 + 4 * speedsCount;
    uint16_t buildTimeIn10us = (buffer[offset] << 8) + buffer[offset + 1];
    uint32_t buildTimeInUs = ((uint32_t)buildTimeIn10us) * 10;

    Serial.printf("CRC8: %d ; Length: %d ; Marker: %d ; Header: %d ; Position1: %d ; Speeds1: [%d, %d, %d, %d, %d, ...]; Position2: %d ; Speeds2: [%d, %d, %d, %d, %d, ...] ; buildTime: %dµs\n", crc8, length, marker, extraHeader, position1, speed10, speed11, speed12, speed13, speed14, position2, speed20, speed21, speed22, speed23, speed24, buildTimeInUs);
}

void buildDataPayload(uint8_t* buffer) {
    // TODO
}

bool checkMarker(uint8_t* buffer, uint8_t expectedMarker) {
    uint8_t receivedMarker = buffer[2];
    bool validMarker = receivedMarker == expectedMarker;
    if (!validMarker) {
        Serial.printf("MARKER NOT VALID !!! Expected: %d but received: %d \n", expectedMarker, receivedMarker);
        return false;
    }
    //Serial.printf("MARKER IS VALID\n");
    return true;
}

#endif