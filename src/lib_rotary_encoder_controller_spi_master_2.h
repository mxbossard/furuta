#ifndef lib_rotary_encoder_controller_spi_master_h
#define lib_rotary_encoder_controller_spi_master_h

//#include <Arduino.h>
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

void spiMasterProcess(uint8_t* data) {
    // start master transaction
    master.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE2));
    digitalWrite(VSPI_SS, LOW);
    master.transferBytes(spi_master_tx_buf, data, BUFFER_SIZE);
    // Or you can transfer like this
    // for (size_t i = 0; i < BUFFER_SIZE; ++i)
    //     spi_master_rx_buf[i] = master.transfer(spi_master_tx_buf[i]);
    digitalWrite(VSPI_SS, HIGH);
    master.endTransaction();
}

#endif