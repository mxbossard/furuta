#ifndef lib_controller_test_h
#define lib_controller_test_h

//#include <Arduino.h>

#include <lib_model.h>

bool assertEventCount(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return true;
    }
    //delay(1);
    int32_t expected = simulator.eventCount;
    int32_t result = sensor.eventCount;
    int32_t drift = abs(expected - result);
    
    if (drift > 0) {
        Serial.printf("Bad event count for [%s] (%d step drift) expected %d but got %d \"%s\" !\n", sensor.name, drift, expected, result, message);
        delay(pause);
    }

    return drift == 0;
}

bool assertCount(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return true;
    }
    //delay(1);
    int32_t expected = simulator.counter;
    int32_t result = sensor.counter;
    int32_t drift = abs(expected - result);

    if (drift > 0) {
        Serial.printf("Bad count for [%s] (%d step drift) expected %d but got %d \"%s\" !\n", sensor.name, drift, expected, result, message);
        delay(pause);
    }

    return drift == 0;
}

bool assertPosition(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return true;
    }
    //delay(1);
    uint16_t result = sensor.position;
    uint16_t expected = simulator.position;
    uint16_t drift = abs(expected - result);
    drift = min(drift, (uint16_t) (sensor.maxPosition - drift));

    if (drift > 0) {
        Serial.printf("Bad position for [%s] (%d step drift) expected %d but got %d \"%s\" !\n", sensor.name, drift, expected, result, message);
        delay(pause);
    }

    return drift == 0;
}

bool assertData(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return true;
    }
    bool test = assertPosition(message, sensor, simulator, 0);
    test = assertCount(message, sensor, simulator, 0) && test;
    test = assertEventCount(message, sensor, simulator, 0) && test;

    if (!test) {
        blinkLed();
        delay(pause);
    }

    return test;
}

void testModulo() {
    int16_t a = -1;
    uint16_t b = 4000;
    uint16_t c = absMod16(a, b);
    Serial.printf("%d absMod16 %d = %d\n", a, b, c);

    a = -1;
    c = absMod16(a, b);
    Serial.printf("%d absMod16 %d = %d\n", a, b, c);

    a = -4001;
    c = absMod16(a, b);
    Serial.printf("%d absMod16 %d = %d\n", a, b, c);

    a = 4000;
    c = absMod16(a, b);
    Serial.printf("%d absMod16 %d = %d\n", a, b, c);

    a = 4001;
    c = absMod16(a, b);
    Serial.printf("%d absMod16 %d = %d\n", a, b, c);
}

void testPendulum(uint16_t amplitude1, uint16_t amplitude2, uint16_t bounces, uint32_t periodInUs) {
    indexSimul(simul1, periodInUs);
    indexSimul(simul2, periodInUs);
    for (; bounces > 0 ; bounces --) {
        moveBothSimulators(true, amplitude1, false, amplitude2, periodInUs);
        // char message[60];
        // sprintf(message, "Pendulum rising bounce %d", bounces);
        // assertPosition(message, simul1, amplitude1);
        // assertPosition(message, simul2, amplitude2);

        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        // sprintf(message, "Pendulum falling bounce %d", bounces);
        // assertPosition(message, sensor1, simul1);
        // assertPosition(message, sensor2, simul2);

        indexSimul(simul1, periodInUs);
        // assertCount("Reseting index", sensor1, simul1);
        indexSimul(simul2, periodInUs);
        // assertCount("Reseting index", sensor2, simul2);

        amplitude1 -= (1/bounces) * amplitude1;
        amplitude2 -= (1/bounces) * amplitude2;
        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        // sprintf(message, "Pendulum rising back bounce %d", bounces);
        // assertPosition(message, sensor1, simul1);
        // assertPosition(message, sensor2, simul2);

        moveBothSimulators(false, amplitude1, true, amplitude2, periodInUs);
        // sprintf(message, "Pendulum falling back bounce %d", bounces);
        // assertPosition(message, sensor1, simul1);
        // assertPosition(message, sensor2, simul2);

        indexSimul(simul1, periodInUs);
        // assertCount("Reseting index", sensor1, simul1);
        // indexSimul(simul2, periodInUs);
        // assertCount("Reseting index", sensor2, simul2);
    }
}

#endif