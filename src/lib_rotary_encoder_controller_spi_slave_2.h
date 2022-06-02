#ifndef lib_rotary_encoder_controller_spi_slave_h
#define lib_rotary_encoder_controller_spi_slave_h

#include <lib_rotary_encoder_controller.h>

#define SOC_SPI_MAXIMUM_BUFFER_SIZE 128
#define SPI_DMA_DISABLED 1
#include <ESP32SPISlave.h>

static constexpr uint8_t VSPI_SS {SS};  // default: GPIO 5
ESP32SPISlave slave;

static const uint32_t BUFFER_SIZE {SPI_WORD_SIZE};
DMA_ATTR uint8_t spi_slave_tx_buf[BUFFER_SIZE];
DMA_ATTR uint8_t spi_slave_rx_buf[BUFFER_SIZE];

DMA_ATTR uint8_t spi_slave_tx_empty[BUFFER_SIZE];
DMA_ATTR uint8_t spi_slave_rx_empty[BUFFER_SIZE];

//uint8_t datagramBuffer[BUFFER_SIZE];

void set_buffer(uint8_t* buff, const size_t size) {
    memset(buff, 0, size);
}

void spiSlaveSetup() {
    set_buffer(spi_slave_tx_buf, BUFFER_SIZE);
    set_buffer(spi_slave_rx_buf, BUFFER_SIZE);

    set_buffer(spi_slave_tx_empty, BUFFER_SIZE);
    set_buffer(spi_slave_rx_empty, BUFFER_SIZE);

    //set_buffer(datagramBuffer, BUFFER_SIZE);

    slave.setDataMode(SPI_MODE0);
    slave.begin(VSPI, SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);

    delay(2000);
}

uint8_t state = 0, marker = 0;

void spiSlaveProcess() {
    if (slave.remained() == 0) {
        // if there is no transaction in queue, add a transaction
        //Serial.printf("State: %d\n", state);

        if (state == COMMAND_TIMING) {
            int64_t startTime = esp_timer_get_time();

            // Master asked for a Timing
            size_t pos = buildDatagram(spi_slave_tx_buf, marker);

            // Add elapsed time in payload
            int64_t endTime = esp_timer_get_time();
            int64_t buildTime64 = (endTime - startTime) / 10;
            uint16_t shortenTime = int64toInt16(buildTime64);
            spi_slave_tx_buf[pos] = shortenTime >> 8;
            spi_slave_tx_buf[pos+1] = shortenTime;

            slave.queue(spi_slave_rx_buf, spi_slave_tx_buf, BUFFER_SIZE);

            uint32_t buildTime32 = ((uint32_t) buildTime64) * 10;
            Serial.printf("Built Payload marked #%d in %d µs.\n", marker, buildTime32);
            state = COMMAND_READ;
        } else if (state == COMMAND_READ) {
            slave.queue(spi_slave_rx_buf, spi_slave_tx_buf, BUFFER_SIZE);
        } else {
            slave.queue(spi_slave_rx_buf, spi_slave_tx_empty, BUFFER_SIZE);
        }

    }

    // if transaction has completed from master,
    // available() returns size of results of transaction,
    // and buffer is automatically updated

    while (slave.available()) {
        // do something with `spi_slave_rx_buf`

        //printCommandPayload(spi_slave_rx_buf);

        uint8_t receivedCommand = getRedundantCommandPayload(spi_slave_rx_buf, SPI_COMMAND_REDUNDANCY);
        Serial.printf("Received command! %d.\n", receivedCommand);
        if (receivedCommand > 0) {
            printCommandPayload(spi_slave_rx_buf);
            state = receivedCommand;

            uint8_t receivedMarker = spi_slave_rx_buf[2];
            uint8_t extraHeader = spi_slave_rx_buf[3];

            if (receivedCommand == COMMAND_TIMING) {
                marker = receivedMarker;
            } else if (receivedCommand == COMMAND_READ) {
                // Master asked for a READ
            }
        } else {
            //state = 0;
            //Serial.println("Unable to decode redundant command !");
            blinkLed();
        }

        slave.pop();
        //Serial.printf("Remaining: %d\n", slave.remained());
    }
    // Wait some time between available() and remaining(). If not, remaining not refreshed().
    delayMicroseconds(10);

    //TODO: use crc16. Check if delay resolve problem.
}

#endif