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
    size_t available;
    size_t avail0, avail1, avail2, avail3, avail4, avail5;
    size_t remain0, remain1, remain2, remain3, remain4, remain5;
    while (available = slave.available(), available) {
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

        avail0 = slave.available();
        remain0 = slave.remained();
        delayMicroseconds(10);
        avail1 = slave.available();
        remain1 = slave.remained();
        delayMicroseconds(1000);
        avail2 = slave.available();
        remain2 = slave.remained();
        slave.pop();
        avail3 = slave.available();
        remain3 = slave.remained();
        delayMicroseconds(10);
        avail4 = slave.available();
        remain4 = slave.remained();
        delayMicroseconds(1000);
        avail5 = slave.available();
        remain5 = slave.remained();

        Serial.printf("slave.avalable(): available: %d ; avail0: %d ; avail1: %d ; avail2: %d ; avail3: %d ; avail4: %d ; avail5: %d \n", available, avail0, avail1, avail2, avail3, avail4, avail5);
        Serial.printf("slave.remained(): remain0: %d ; remain1: %d ; remain2: %d ; remain3: %d ; remain4: %d ; remain5: %d \n", remain0, remain1, remain2, remain3, remain4, remain5);
        
        //Serial.printf("Remaining: %d\n", slave.remained());
    }
    // Wait some time between available() and remaining(). If not, remaining not refreshed().
    delayMicroseconds(10);

    //TODO: use crc16. Check if delay resolve problem.
}

#endif