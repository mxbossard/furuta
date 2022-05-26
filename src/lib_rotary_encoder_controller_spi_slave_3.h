#ifndef lib_rotary_encoder_controller_spi_slave_h
#define lib_rotary_encoder_controller_spi_slave_h

#include <Arduino.h>
#include <SlaveSPI.h>
#include <SPI.h>

#include <lib_rotary_encoder_controller.h>

SPISettings spi_setting(1000000, MSBFIRST, SPI_MODE);
SlaveSPI slave(HSPI_HOST);  // VSPI_HOST

#include "SimpleArray.h"
typedef SimpleArray<uint8_t, int> array_t;

array_t master_msg(SPI_DEFAULT_MAX_BUFFER_SIZE);
array_t slave_msg(SPI_DEFAULT_MAX_BUFFER_SIZE);

void printHex(array_t arr) {
    for (int i = 0; i < arr.length(); i++) {
        Serial.print(arr[i], HEX);
        Serial.print(" ");
    }
}

void printlnHex(array_t arr) {
    printHex(arr);
    Serial.println();
}

int callback_after_slave_tx_finish() {
    // Serial.println("[slave_tx_finish] slave transmission has been finished!");
    // Serial.println(slave[0]);

    return 0;
}

void spiSlaveSetup() {
    // Setup Slave-SPI
    // slave.begin(SO, SI, SCLK, SS, 8, callback_after_slave_tx_finish);  // seems to work with groups of 4 bytes
    // slave.begin(SO, SI, SCLK, SS, 4, callback_after_slave_tx_finish);
    slave.begin(SPI_MISO, SPI_MOSI, SPI_SCLK, SPI_CS, SPI_WORD_SIZE, callback_after_slave_tx_finish);
    // slave.begin(SO, SI, SCLK, SS, 1, callback_after_slave_tx_finish);  // at least 2 word in an SPI frame
}

void spiSlaveProcess() {
   
}

#endif