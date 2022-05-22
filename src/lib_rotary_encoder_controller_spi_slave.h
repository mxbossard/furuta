#ifndef lib_rotary_encoder_controller_spi_slave_h
#define lib_rotary_encoder_controller_spi_slave_h

#include <ESP32SPISlave.h>

#include <lib_rotary_encoder_controller.h>

#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_SCLK 18
#define SPI_CS   5
#define SPI_FREQUENCY 10000

ESP32SPISlave slave;

static constexpr uint32_t BUFFER_SIZE {64};
uint8_t spi_slave_tx_buf[BUFFER_SIZE];
uint8_t spi_slave_rx_buf[BUFFER_SIZE];

void spiSlaveSetup() {
    // HSPI = CS: 15, CLK: 14, MOSI: 13, MISO: 12 -> default
    // VSPI = CS:  5, CLK: 18, MOSI: 23, MISO: 19
    slave.setDataMode(SPI_MODE0);
    slave.begin(HSPI);
}

void spiSlaveProcess() {
    // block until the transaction comes from master
    slave.wait(spi_slave_rx_buf, spi_slave_tx_buf, BUFFER_SIZE);

    // if transaction has completed from master,
    // available() returns size of results of transaction,
    // and `spi_slave_rx_buf` is automatically updated
    while (slave.available()) {
        // do something with `spi_slave_rx_buf`

        sensorsMessage(sensorsMessageBuffer);
        memcpy(&spi_slave_tx_buf, sensorsMessageBuffer, sizeof(float));

        slave.pop();
    }
}

#endif