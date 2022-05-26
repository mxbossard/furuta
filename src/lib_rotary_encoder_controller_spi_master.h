#ifndef lib_rotary_encoder_controller_spi_master_h
#define lib_rotary_encoder_controller_spi_master_h

//#include <Arduino.h>
//#include <SPI.h>

#define SPI_DMA_CH_AUTO 3
#define SPI_TRANS_VARIABLE_DUMMY (1<<7)
#include <ESP32DMASPIMaster.h>

ESP32DMASPI::Master master;

static const uint32_t BUFFER_SIZE = SPI_WORD_SIZE;
uint8_t* spi_master_tx_buf;
uint8_t* spi_master_rx_buf;

// static const int spiClk = 240000000; // 1 MHz
// SPIClass * hspi = NULL;

void spiMasterSetup() {
    // to use DMA buffer, use these methods to allocate buffer
    spi_master_tx_buf = master.allocDMABuffer(BUFFER_SIZE);
    spi_master_rx_buf = master.allocDMABuffer(BUFFER_SIZE);

    // set buffer data...

    master.setDataMode(SPI_MODE0);           // default: SPI_MODE0
    //master.setFrequency(4000000);            // default: 8MHz (too fast for bread board...)
    master.setFrequency(SPI_FREQUENCY);
    master.setMaxTransferSize(BUFFER_SIZE);  // default: 4092 bytes

    // begin() after setting
    master.begin();  // HSPI (CS: 15, CLK: 14, MOSI: 13, MISO: 12) -> default
                     // VSPI (CS:  5, CLK: 18, MOSI: 23, MISO: 19)
}

bool calcEvenParity(uint16_t payload) {
    //Serial.printf("payload: 0x%04x ; ", payload);

    // Exclude parity bit (Most Significatif Bit)
    byte bitCount = sizeof(payload) * 8;
    byte cnt = 0;
	byte i;

	for (i = 0; i < bitCount; i++) {
		if (payload & 0x1) {
			cnt ++;
		}
		payload >>= 1;
	}

    // Return 1 if odd number of 1 in payload
    bool result = cnt & 0x1;
    //Serial.printf("bitCount: %d ; parity: %d\n", bitCount, result);
    return result;
}

uint16_t paritize(uint16_t payload) {
    bool parity = calcEvenParity(payload);
    return payload | (parity << 15);
}

void spiMasterProcess(uint8_t* data) {
    //master.transfer(spi_master_tx_buf, spi_master_rx_buf, BUFFER_SIZE);
    master.transfer(NULL, data, SPI_WORD_SIZE);

    // Read SPI data
    // SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
    // SPI.transferBytes(NULL, data, SPI_WORD_SIZE);
    // SPI.endTransaction();
}

#endif