#include <Arduino.h>

#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_CLK 18
#define SPI_CS   5

#define SIMUL_1_PIN_A 16
#define SIMUL_1_PIN_B 4
#define SIMUL_1_PIN_INDEX 0

#define SIMUL_2_PIN_A 27
#define SIMUL_2_PIN_B 14
#define SIMUL_2_PIN_INDEX 12

#define LED_PIN 22

#define LOG_WARN
#define LOG_INFO

#include <rotary_encoder_config.h>
#include <lib_rotary_encoder_simulator_2.h>
#include <lib_rotary_encoder_controller_spi_master_2.h>
#include <lib_simulator_test_2.h>
#include <lib_datagram.h>

void setup() {
    Serial.begin(115200);

    //simulatorSetup();
    spiMasterSetup();

    pinMode(LED_PIN, OUTPUT);
}

uint8_t* messageBuffer = (uint8_t*) malloc(sizeof(uint8_t) * SPI_WORD_SIZE);

void loop() {
    // delay(2000);

    Serial.println("Running simulation ...");
    int64_t startTime = esp_timer_get_time();
    // testModulo();

    // printSimulators();
    // printSensors();

    uint32_t periodInUs = 40;
    uint32_t failedTestPause = 0;

    rs1.index(periodInUs);
    rs2.index(periodInUs);
    assertSpiPayload("Indexing sensors", failedTestPause);

    moveBothSimulators(true, 0, false, 1, periodInUs);
    assertSpiPayload("Turning 1 step left (s2)", failedTestPause);

    indexSimul(&simul1, periodInUs);
    indexSimul(&simul2, periodInUs);
    moveBothSimulators(true, 1, false, 0, periodInUs);
    assertSpiPayload("Turning 1 step right (s1)", failedTestPause);

    indexSimul(&simul1, periodInUs);
    indexSimul(&simul2, periodInUs);
    moveBothSimulators(true, 1, false, 1, periodInUs);
    assertSpiPayload("Turning 1 step right (s1) and 1 step left (s2)", failedTestPause);

    indexSimul(&simul1, periodInUs);
    indexSimul(&simul2, periodInUs);
    moveBothSimulators(false, 2, true, 3, periodInUs);
    assertSpiPayload("Turning 2 step left (s1) and 3 step right (s2)", failedTestPause);

    moveBothSimulators(true, 3, false, 2, periodInUs);
    assertSpiPayload("Turning 3 step right (s1) and 2 step left (s2)", failedTestPause);

    indexSimul(&simul1, periodInUs);
    indexSimul(&simul2, periodInUs);
    moveBothSimulators(true, 11, false, 7, periodInUs);
    assertSpiPayload("Turning 11 steps right (s1) and 7 steps left (s2)", failedTestPause);

    indexSimul(&simul1, periodInUs);
    indexSimul(&simul2, periodInUs);
    moveBothSimulators(true, 101, true, 101, periodInUs);
    assertSpiPayload("Turning 101 steps right (s1) and 101 steps right (s2)", failedTestPause);

    indexSimul(&simul1, periodInUs);
    indexSimul(&simul2, periodInUs);
    moveBothSimulators(true, 3997, false, 1001, periodInUs);
    assertSpiPayload("Turning 3997 steps right (s1) and 1001 steps left (s2)", failedTestPause);
   
    // Turn 1 round one side
    indexSimul(&simul1, periodInUs);
    indexSimul(&simul2, periodInUs);
    moveBothSimulators(true, 4003, false, 995, periodInUs);
    assertSpiPayload("Turning 4003 steps right (s1) and 995 steps left (s2)", failedTestPause);

    testPendulum(2000, 500, 12, periodInUs, failedTestPause);

    moveBothSimulators(true, 3, false, 3, periodInUs);
    assertSpiPayload("Turning 3 step right (s1) and 2 step left (s2)", failedTestPause);

    // Print last data payload
    printDataPayload(spi_master_rx_buf, SPEEDS_COUNT_TO_KEEP);

    int64_t endTime = esp_timer_get_time();
    int32_t duration = (int32_t) (endTime - startTime);

    Serial.printf("Simulation took %d µs.\n\n", duration);
    //delay(2000);
}