#ifndef lib_rotary_encoder_controller_spi_master_h
#define lib_rotary_encoder_controller_spi_master_h

//#include <Arduino.h>
//#include <SPI.h>

#define SPI_DMA_CH_AUTO 0
#define SPI_TRANS_VARIABLE_DUMMY (1<<7)
#include <ESP32DMASPIMaster.h>

ESP32DMASPI::Master master;

static const uint32_t BUFFER_SIZE = SPI_WORD_SIZE;
uint8_t* spi_master_tx_buf;
uint8_t* spi_master_rx_buf;

// static const int spiClk = 240000000; // 1 MHz
// SPIClass * hspi = NULL;

void set_buffer(uint8_t* buff, const size_t size) {
    memset(buff, 0, size);
}

void spiMasterSetup() {

    // to use DMA buffer, use these methods to allocate buffer
    spi_master_tx_buf = master.allocDMABuffer(BUFFER_SIZE);
    spi_master_rx_buf = master.allocDMABuffer(BUFFER_SIZE);

    // set buffer data...
    set_buffer(spi_master_tx_buf, BUFFER_SIZE);
    set_buffer(spi_master_rx_buf, BUFFER_SIZE);
    delay(5000);

    master.setDataMode(SPI_MODE0);           // default: SPI_MODE0
    //master.setFrequency(4000000);            // default: 8MHz (too fast for bread board...)
    master.setFrequency(SPI_FREQUENCY);
    master.setMaxTransferSize(BUFFER_SIZE);  // default: 4092 bytes
    master.setDutyCyclePos(96);   

    // begin() after setting
    master.begin(VSPI);
    //master.begin(VSPI, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);  // HSPI (CS: 15, CLK: 14, MOSI: 13, MISO: 12) -> default
                     // VSPI (CS:  5, CLK: 18, MOSI: 23, MISO: 19)
}

void spiMasterProcess(uint8_t* data) {
    //master.transfer(spi_master_tx_buf, spi_master_rx_buf, BUFFER_SIZE);
    master.transfer(spi_master_tx_buf, data, SPI_WORD_SIZE);

    // Read SPI data
    // SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
    // SPI.transferBytes(NULL, data, SPI_WORD_SIZE);
    // SPI.endTransaction();
}

#endif