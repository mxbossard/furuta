#ifndef lib_controller_test_h
#define lib_controller_test_h

//#include <Arduino.h>

#include <lib_model.h>

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

void assertPosition(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
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

void testPendulum(uint16_t amplitude1, uint16_t amplitude2, uint16_t bounces, uint32_t periodInUs) {
    indexSimul(simul1, periodInUs);
    indexSimul(simul2, periodInUs);
    for (; bounces > 0 ; bounces --) {
        moveBothSimulators(true, amplitude1, false, amplitude2, periodInUs);
        char message[60];
        sprintf(message, "Pendulum rising bounce %d", bounces);
        assertPosition(message, sensor1, simul1);
        assertPosition(message, sensor2, simul2);

        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        sprintf(message, "Pendulum falling bounce %d", bounces);
        assertPosition(message, sensor1, simul1);
        assertPosition(message, sensor2, simul2);

        indexSimul(simul1, periodInUs);
        assertCount("Reseting index", sensor1, simul1);
        indexSimul(simul2, periodInUs);
        assertCount("Reseting index", sensor2, simul2);

        amplitude1 -= (1/bounces) * amplitude1;
        amplitude2 -= (1/bounces) * amplitude2;
        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        sprintf(message, "Pendulum rising back bounce %d", bounces);
        assertPosition(message, sensor1, simul1);
        assertPosition(message, sensor2, simul2);

        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        sprintf(message, "Pendulum falling back bounce %d", bounces);
        assertPosition(message, sensor1, simul1);
        assertPosition(message, sensor2, simul2);

        indexSimul(simul1, periodInUs);
        assertCount("Reseting index", sensor1, simul1);
        indexSimul(simul2, periodInUs);
        assertCount("Reseting index", sensor2, simul2);
    }
}

#endif