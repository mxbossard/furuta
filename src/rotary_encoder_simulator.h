#include <Arduino.h>

#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_CLK 18
#define SPI_CS   5
#define SPI_FREQUENCY 100000
#define SPI_WORD_SIZE 32

#define SIMUL_1_PIN_A 32
#define SIMUL_1_PIN_B 33
#define SIMUL_1_PIN_INDEX 25

#define SIMUL_2_PIN_A 27
#define SIMUL_2_PIN_B 14
#define SIMUL_2_PIN_INDEX 12

#define LED_PIN 22

#include <lib_rotary_encoder_simulator.h>
#include <lib_rotary_encoder_controller_spi_master_2.h>

void setup() {
    Serial.begin(115200);

    //simulatorSetup();
    spiMasterSetup();

    pinMode(LED_PIN, OUTPUT);
}

int counter = 0;
uint8_t* messageBuffer = (uint8_t*) malloc(sizeof(uint8_t) * SPI_WORD_SIZE);

void loop() {
    delay(1000);

    spiMasterProcess(messageBuffer);
    Serial.printf("Received data: ");
    for (int32_t k = 0; k < SPI_WORD_SIZE; k++) {
        Serial.printf("%c ", messageBuffer[k]);
    }
    Serial.println();
 
    // Blink led
    counter ++;
    digitalWrite(LED_PIN, counter % 2);
}