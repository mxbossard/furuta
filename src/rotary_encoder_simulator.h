//#include <Arduino.h>

#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_SCLK 18
#define SPI_CS   5
#define SPI_FREQUENCY 100000
#define SPI_WORD_SIZE 128

#define SIMUL_1_PIN_A 36
#define SIMUL_1_PIN_B 39
#define SIMUL_1_PIN_INDEX 34

#define SIMUL_2_PIN_A 25
#define SIMUL_2_PIN_B 26
#define SIMUL_2_PIN_INDEX 27

#define LED_PIN 21

#include <lib_rotary_encoder_simulator.h>
#include <lib_rotary_encoder_controller_spi_master.h>

void setup() {
    Serial.begin(115200);

    simulatorSetup();
    spiMasterSetup();

    pinMode(LED_PIN, OUTPUT);
}

int counter = 0;
uint8_t* messageBuffer = (uint8_t*) malloc(sizeof(uint8_t) * SPI_WORD_SIZE);

void loop() {
    delay(1000);

    spiMasterProcess(messageBuffer);
    Serial.printf("Received data: %s\n", messageBuffer);

    // Blink led
    counter ++;
    digitalWrite(LED_PIN, counter % 2);
}