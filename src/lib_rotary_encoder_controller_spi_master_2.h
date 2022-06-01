#ifndef lib_rotary_encoder_controller_spi_master_h
#define lib_rotary_encoder_controller_spi_master_h

#include <Arduino.h>
#include <SPI.h>

#include <lib_utils.h>
#include <lib_datagram.h>

static constexpr uint8_t VSPI_SS {5};  // default: GPIO 5
SPIClass master(VSPI);

static const uint32_t BUFFER_SIZE {SPI_WORD_SIZE};
uint8_t spi_master_tx_buf[BUFFER_SIZE];
uint8_t spi_master_rx_buf[BUFFER_SIZE];

void set_buffer(uint8_t* buff, const size_t size) {
    memset(buff, 0, size);
}

void spiMasterSetup() {
    set_buffer(spi_master_tx_buf, SPI_WORD_SIZE);
    set_buffer(spi_master_rx_buf, SPI_WORD_SIZE);

    // SPI Master
    // VSPI = CS: 5, CLK: 18, MOSI: 23, MISO: 19
    pinMode(VSPI_SS, OUTPUT);
    pinMode(SPI_CLK, OUTPUT);
    digitalWrite(VSPI_SS, HIGH);
    master.begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
}

void sendSpiTransaction(uint8_t* txBuffer, uint8_t* rxBuffer, size_t length) {
    // start master transaction
    master.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE2));
    digitalWrite(VSPI_SS, LOW);
    master.transferBytes(txBuffer, rxBuffer, length);
    digitalWrite(VSPI_SS, HIGH);
    master.endTransaction();
}

const size_t commandLength = 5;
uint8_t timingMarker = 0;

bool sendSpiTimingCommand() {
    timingMarker ++;

    buildCommandPayload(spi_master_tx_buf, timingMarker, COMMAND_TIMING);
    
    // MBD: Transfer 10 extra byte because it seems the last byte is not trasnfered !!!
    sendSpiTransaction(spi_master_tx_buf, spi_master_rx_buf, commandLength + 10);

    return false;
}

bool sendSpiReadCommand() {
    buildCommandPayload(spi_master_tx_buf, timingMarker, COMMAND_READ);
    
    sendSpiTransaction(spi_master_tx_buf, spi_master_rx_buf, BUFFER_SIZE);

    // Check CRC8
    bool validCrc = checkDataCrc8(spi_master_rx_buf);
    // Check marker
    bool validMarker = checkMarker(spi_master_rx_buf, timingMarker);

    return validCrc && validMarker;
}

uint8_t* spiMasterProcess() {
    sendSpiTimingCommand();

    delay(2);

    int8_t retries = 0;
    while(!sendSpiReadCommand() && retries < 5) {
        delay(2);
        retries ++;
        Serial.printf("Retrying sendSpiReadCommand() #%d ...\n", retries);
    };
    printDataPayload(spi_master_rx_buf, SPEEDS_COUNT_TO_KEEP);

    return spi_master_rx_buf;
}

#endif