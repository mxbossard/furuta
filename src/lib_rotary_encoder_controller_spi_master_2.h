#ifndef lib_rotary_encoder_controller_spi_master_h
#define lib_rotary_encoder_controller_spi_master_h

#include <Arduino.h>
#include <SPI.h>

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

uint8_t timingMarker = 0;
bool spiMasterProcess(uint8_t* data, uint8_t command) {
    spi_master_tx_buf[1] = 5; // length
    spi_master_tx_buf[2] = timingMarker; // marker
    spi_master_tx_buf[3] = 0; // extraHeader
    spi_master_tx_buf[4] = command; // command
    markCrc8(spi_master_tx_buf);

    if (command == COMMAND_TIMING) {
        timingMarker ++;
    }

    // start master transaction
    master.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE2));
    digitalWrite(VSPI_SS, LOW);
    master.transferBytes(spi_master_tx_buf, data, BUFFER_SIZE);
    // Or you can transfer like this
    // for (size_t i = 0; i < BUFFER_SIZE; ++i)
    //     spi_master_rx_buf[i] = master.transfer(spi_master_tx_buf[i]);
    digitalWrite(VSPI_SS, HIGH);
    master.endTransaction();

    // Check CRC8
    bool validCrc = checkCrc8(data);

    // Check marker
    bool validMarker = true;
    if (command == COMMAND_READ) {
        uint8_t receivedMarker = data[2];
        validMarker = receivedMarker == timingMarker;
        if (!validMarker) {
            Serial.printf("MARKER NOT VALID !!! Expected: %d but received: %d \n", timingMarker, receivedMarker);
        }
    }
    
    return validCrc && validMarker;
}

#endif