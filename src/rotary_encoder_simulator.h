#include <Arduino.h>

#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_CLK 18
#define SPI_CS   5

#define SIMUL_1_PIN_A 32
#define SIMUL_1_PIN_B 33
#define SIMUL_1_PIN_INDEX 25

#define SIMUL_2_PIN_A 27
#define SIMUL_2_PIN_B 14
#define SIMUL_2_PIN_INDEX 12

#define LED_PIN 22

#include <rotary_encoder_config.h>
#include <lib_rotary_encoder_simulator.h>
#include <lib_rotary_encoder_controller_spi_master_2.h>
#include <lib_controller_test.h>
#include <lib_datagram.h>

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

    Serial.println("Running simulation ...");
    // testModulo();

    // printSimulators();
    // printSensors();

    uint32_t periodInUs = 10;

    indexSimul(simul1, periodInUs);
    // assertCount("Reseting index", sensor1, simul1);
    indexSimul(simul2, periodInUs);
    // assertCount("Reseting index", sensor2, simul2);
    // printSimulators();
    // printSensors();

    moveBothSimulators(true, 0, false, 1, periodInUs);
    // assertCount("Turning 0 steps", sensor1, simul1);
    // assertCount("Turning 1 step left", sensor2, simul2);

    moveBothSimulators(true, 1, false, 0, periodInUs);
    // assertCount("Turning 1 steps right", sensor1, simul1);
    // assertCount("Turning 0 step", sensor2, simul2);

    moveBothSimulators(true, 1, false, 1, periodInUs);
    // assertCount("Turning 1 steps right", sensor1, simul1);
    // assertCount("Turning 1 step left", sensor2, simul2);

    //printSensors();

    moveBothSimulators(false, 2, true, 3, periodInUs);
    // assertCount("Turning 2 steps left", sensor1, simul1);
    // assertCount("Turning 3 step right", sensor2, simul2);

    //printSensors();

    moveBothSimulators(true, 3, false, 2, periodInUs);
    // assertCount("Turning 3 steps right", sensor1, simul1);
    // assertCount("Turning 2 step left", sensor2, simul2);

    moveBothSimulators(true, 11, false, 7, periodInUs);
    // assertCount("Turning 11 steps right", sensor1, simul1);
    // assertCount("Turning 7 steps left", sensor2, simul2);
    
    spiMasterProcess(messageBuffer);
    decodeDatagram(messageBuffer, SPI_WORD_SIZE);

    moveBothSimulators(true, 101, true, 101, periodInUs);
    // assertCount("Turning 101 steps right", sensor1, simul1);
    // assertCount("Turning 101 steps right", sensor2, simul2);

    moveBothSimulators(true, 3997, false, 1001, periodInUs);
    // assertCount("Turning 3997 steps right", sensor1, simul1);
    // assertCount("Turning 1001 steps left", sensor2, simul2);
   
    // Turn 1 round one side
    moveBothSimulators(true, 4003, false, 995, periodInUs);
    // assertCount("Turning 1 round + 3 steps right", sensor1, simul1);
    // assertCount("Turning 1 round - 5 steps right", sensor2, simul2);
    // printSimulators();
    // printSensors();

    testPendulum(2000, 500, 12, periodInUs);

    spiMasterProcess(messageBuffer);
    decodeDatagram(messageBuffer, SPI_WORD_SIZE);
 
    // Blink led
    counter ++;
    digitalWrite(LED_PIN, counter % 2);
}