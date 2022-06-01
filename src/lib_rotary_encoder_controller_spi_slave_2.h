#ifndef lib_rotary_encoder_controller_spi_slave_h
#define lib_rotary_encoder_controller_spi_slave_h

#include <lib_rotary_encoder_controller.h>

#define SOC_SPI_MAXIMUM_BUFFER_SIZE 128
#define SPI_DMA_DISABLED 1
#include <ESP32SPISlave.h>

static constexpr uint8_t VSPI_SS {SS};  // default: GPIO 5
ESP32SPISlave slave;

static const uint32_t BUFFER_SIZE {SPI_WORD_SIZE};
uint8_t spi_slave_tx_buf[BUFFER_SIZE];
uint8_t spi_slave_rx_buf[BUFFER_SIZE];

uint8_t datagramBuffer[BUFFER_SIZE];

void set_buffer(uint8_t* buff, const size_t size) {
    memset(buff, 0, size);
}

void spiSlaveSetup() {
    set_buffer(spi_slave_tx_buf, SPI_WORD_SIZE);
    set_buffer(spi_slave_rx_buf, SPI_WORD_SIZE);
    //set_buffer(datagramBuffer, SPI_WORD_SIZE);

    slave.setDataMode(SPI_MODE0);
    slave.begin(VSPI, SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);

    delay(2000);
}

int counter = 0;
void spiSlaveProcess() {
    if (slave.remained() == 0) {
        // if there is no transaction in queue, add a transaction
        slave.queue(spi_slave_rx_buf, spi_slave_tx_buf, BUFFER_SIZE);
        printDataPayload(spi_slave_tx_buf, SPEEDS_COUNT_TO_KEEP);

        // Blink led
        counter ++;
        digitalWrite(LED_PIN, counter % 2);
    }

    // if transaction has completed from master,
    // available() returns size of results of transaction,
    // and buffer is automatically updated

    while (slave.available()) {
        slave.pop();
        
        int64_t startTime = esp_timer_get_time();
        printCommandPayload(spi_slave_rx_buf);

        bool validCrc = checkCommandCrc8(spi_slave_rx_buf);
        if (validCrc) {
            uint8_t length = spi_slave_rx_buf[1];
            uint8_t marker = spi_slave_rx_buf[2];
            uint8_t extraHeader = spi_slave_rx_buf[3];
            uint8_t command = spi_slave_rx_buf[4];

            if (command == COMMAND_TIMING) {
                // Master asked for a Timing
                size_t pos = buildDatagram(spi_slave_tx_buf, marker);

                // Add elapsed time in payload
                int64_t endTime = esp_timer_get_time();
                int64_t buildTime = (endTime - startTime) / 10;
                uint16_t shortenTime = int64toInt16(buildTime);
                spi_slave_tx_buf[pos] = shortenTime >> 8;
                spi_slave_tx_buf[pos+1] = shortenTime;

            } else if (command == COMMAND_READ) {
                // Master asked for a READ
            }
        }

    }
}

#endif