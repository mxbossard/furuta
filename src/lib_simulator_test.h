#ifndef lib_controller_test_h
#define lib_controller_test_h

//#include <Arduino.h>

#include <lib_model.h>
#include <lib_utils.h>
#include <lib_datagram.h>
#include <lib_rotary_encoder_simulator.h>

void assertCount(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return;
    }
    //delay(1);
    int32_t expectedCount = simulator.position;
    int32_t drift = abs(expectedCount - sensor.position);

    if (sensor.position != expectedCount) {
        Serial.printf("Bad count for [%s] (%d step drift) expected %d but got %d \"%s\" !\n", sensor.name, drift, expectedCount, sensor.position, message);
        delay(pause);
    }
}

void assertSpiPayload(const char* message, uint32_t pause = 0) {
    delayMicroseconds(100);
    bool valid = spiMasterProcess();
    if (valid) {
        //printDataPayload(spi_master_rx_buf, SPEEDS_COUNT_TO_KEEP);
    } else {
        blinkLed();
        Serial.printf("Invalid payload received for test: \"%s\" !\n", message);
        printDataPayload(spi_master_rx_buf, SPEEDS_COUNT_TO_KEEP);
        return;
    }

    uint16_t expectedPosition1 = (uint16_t) absMod32(simul1.position, simul1.maxPosition);
    uint16_t expectedPosition2 = (uint16_t) absMod32(simul2.position, simul2.maxPosition);

    uint16_t position1 = getPosition1(spi_master_rx_buf);
    uint16_t position2 = getPosition2(spi_master_rx_buf);

    bool doPause = false;
    if (position1 != expectedPosition1) {
        int32_t drift = abs(expectedPosition1 - position1);
        drift = min(drift, simul1.maxPosition - drift);
        Serial.printf("Bad position for sensor1 (%d step drift) expected %d but got %d \"%s\" !\n", drift, expectedPosition1, position1, message);
        doPause = true;
    }
    if (position2 != expectedPosition2) {
        int32_t drift = abs(expectedPosition2 - position2);
        drift = min(drift, simul2.maxPosition - drift);
        Serial.printf("Bad position for sensor2 (%d step drift) expected %d but got %d \"%s\" !\n", drift, expectedPosition2, position2, message);
        doPause = true;
    }

    if (doPause) {
        delay(pause);
    }
}

void assertPosition(const char* message, AngleSensorSimulator simulator, AngleSensor sensor, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return;
    }
    //delay(1);
    int32_t position = absMod32(sensor.position, sensor.maxPosition);
    int32_t expectedPosition = absMod32(simulator.position, sensor.maxPosition);
    int32_t drift = abs(expectedPosition - position);
    drift = min(drift, sensor.maxPosition - drift);

    if (position != expectedPosition) {
        Serial.printf("Bad position for [%s] (%d step drift) expected %d but got %d \"%s\" !\n", sensor.name, drift, simulator.position, sensor.position, message);
        delay(pause);
    }
}

void assertPosition(const char* message, AngleSensorSimulator simulator, int32_t currentCount, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return;
    }
    //delay(1);
    int32_t expectedPosition = absMod32(simulator.position, simulator.maxPosition);
    int32_t drift = abs(expectedPosition - currentCount);
    drift = min(drift, simulator.maxPosition - drift);

    if (currentCount != expectedPosition) {
        Serial.printf("Bad position for [%s] (%d step drift) expected %d but got %d \"%s\" !\n", simulator.name, drift, simulator.position, currentCount, message);
        delay(pause);
    }
}



void testModulo() {
    int16_t a = -1;
    uint16_t b = 4000;
    int32_t c = absMod32(a, b);
    Serial.printf("%d absMod32 %d = %d\n", a, b, c);

    a = -1;
    c = absMod32(a, b);
    Serial.printf("%d absMod32 %d = %d\n", a, b, c);

    a = -4001;
    c = absMod32(a, b);
    Serial.printf("%d absMod32 %d = %d\n", a, b, c);

    a = 4000;
    c = absMod32(a, b);
    Serial.printf("%d absMod32 %d = %d\n", a, b, c);

    a = 4001;
    c = absMod32(a, b);
    Serial.printf("%d absMod32 %d = %d\n", a, b, c);
}

void testPendulum(uint16_t amplitude1, uint16_t amplitude2, uint16_t bounces, uint32_t periodInUs, uint32_t failedTestPause) {
    indexSimul(&simul1, periodInUs);
    indexSimul(&simul2, periodInUs);
    for (; bounces > 0 ; bounces --) {
        moveBothSimulators(true, amplitude1, false, amplitude2, periodInUs);
        char message[60];
        sprintf(message, "Pendulum rising bounce %d", bounces);
        assertSpiPayload(message, failedTestPause);

        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        sprintf(message, "Pendulum falling bounce %d", bounces);
        assertSpiPayload(message, failedTestPause);

        indexSimul(&simul1, periodInUs);
        indexSimul(&simul2, periodInUs);
        assertSpiPayload("Indexing sensors", failedTestPause);

        amplitude1 -= (1/bounces) * amplitude1;
        amplitude2 -= (1/bounces) * amplitude2;
        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        sprintf(message, "Pendulum rising back bounce %d", bounces);
        assertSpiPayload(message, failedTestPause);

        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        sprintf(message, "Pendulum falling back bounce %d", bounces);
        assertSpiPayload(message, failedTestPause);

        indexSimul(&simul1, periodInUs);
        indexSimul(&simul2, periodInUs);
        assertSpiPayload("Indexing sensors", failedTestPause);
    }
}

#endif