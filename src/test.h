#include <Arduino.h>

#include <rotary_encoder_controller.h>
#include <rotary_encoder_simulator.h>

void assertCount(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return;
    }
    delay(1);
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
    delay(1);
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

void setup() {
    resetCircularBuffer(&timings1);
    resetCircularBuffer(&timings2);

    Serial.begin(115200);
    //hspi = new SPIClass(HSPI);
    //hspi->begin();
    //SPI.begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS); //SCLK, MISO, MOSI, SS
    //pinMode(HSPI_CS, OUTPUT); //HSPI SS
    //digitalWrite(HSPI_CS, HIGH);

    // Sensor 1
    pinMode(sensor1.pinA, INPUT_PULLUP);
    pinMode(sensor1.pinB, INPUT_PULLUP);
    pinMode(sensor1.pinIndex, INPUT_PULLUP);
    attachInterrupt(sensor1.pinA, moveSensor1A, CHANGE);
    attachInterrupt(sensor1.pinB, moveSensor1B, CHANGE);
    attachInterrupt(sensor1.pinIndex, resetSensor1, RISING);

    // Sensor 2
    pinMode(sensor2.pinA, INPUT_PULLUP);
    pinMode(sensor2.pinB, INPUT_PULLUP);
    pinMode(sensor2.pinIndex, INPUT_PULLUP);
    attachInterrupt(sensor2.pinA, moveSensor2A, CHANGE);
    attachInterrupt(sensor2.pinB, moveSensor2B, CHANGE);
    attachInterrupt(sensor2.pinIndex, resetSensor2, RISING);

    // Simulateur sensor 1
    pinMode(simul1.pinA, OUTPUT);
    pinMode(simul1.pinB, OUTPUT);
    pinMode(simul1.pinIndex, OUTPUT);
    digitalWrite(simul1.pinA, LOW);
    digitalWrite(simul1.pinB, LOW);
    digitalWrite(simul1.pinIndex, LOW);

    // Simulateur sensor 1
    pinMode(simul2.pinA, OUTPUT);
    pinMode(simul2.pinB, OUTPUT);
    pinMode(simul2.pinIndex, OUTPUT);
    digitalWrite(simul2.pinA, LOW);
    digitalWrite(simul2.pinB, LOW);
    digitalWrite(simul2.pinIndex, LOW);

    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    //simul1.enabled = false;
    //simul2.enabled = false;

    delay(100);

    Serial.println("Running simulation ...");
    // testModulo();

    // printSimulators();
    // printSensors();

    uint32_t periodInUs = 1;

    indexSimul(simul1, periodInUs);
    assertCount("Reseting index", sensor1, simul1);
    indexSimul(simul2, periodInUs);
    assertCount("Reseting index", sensor2, simul2);
    // printSimulators();
    // printSensors();

    moveBothSimulators(true, 0, false, 1, periodInUs);
    assertCount("Turning 0 steps", sensor1, simul1);
    assertCount("Turning 1 step left", sensor2, simul2);

    moveBothSimulators(true, 1, false, 0, periodInUs);
    assertCount("Turning 1 steps right", sensor1, simul1);
    assertCount("Turning 0 step", sensor2, simul2);

    moveBothSimulators(true, 1, false, 1, periodInUs);
    assertCount("Turning 1 steps right", sensor1, simul1);
    assertCount("Turning 1 step left", sensor2, simul2);

    printSensors();

    moveBothSimulators(true, 2, false, 3, periodInUs);
    assertCount("Turning 2 steps right", sensor1, simul1);
    assertCount("Turning 3 step left", sensor2, simul2);

    printSensors();

    moveBothSimulators(true, 3, false, 2, periodInUs);
    assertCount("Turning 3 steps right", sensor1, simul1);
    assertCount("Turning 2 step left", sensor2, simul2);

    printSensors();

    moveBothSimulators(true, 11, false, 7, periodInUs);
    assertCount("Turning 11 steps right", sensor1, simul1);
    assertCount("Turning 7 steps left", sensor2, simul2);
    
    moveBothSimulators(true, 101, true, 101, periodInUs);
    assertCount("Turning 101 steps right", sensor1, simul1);
    assertCount("Turning 101 steps right", sensor2, simul2);

    moveBothSimulators(true, 3997, false, 1001, periodInUs);
    assertCount("Turning 3997 steps right", sensor1, simul1);
    assertCount("Turning 1001 steps left", sensor2, simul2);
   
    // Turn 1 round one side
    moveBothSimulators(true, sensor1.maxPosition + 3, false, sensor2.maxPosition - 5, periodInUs);
    assertCount("Turning 1 round + 3 steps right", sensor1, simul1);
    assertCount("Turning 1 round - 5 steps right", sensor2, simul2);
    // printSimulators();
    // printSensors();

    testPendulum(sensor1.maxPosition/2, sensor2.maxPosition/2, 12, periodInUs);

    // Turn 50 round one side
    //moveBothSimulators(true, sensor1.maxPosition * 50, false, sensor2.maxPosition * 50, periodInUs);
    //assertPosition("Turning 50 round right", sensor1, simul1);
    //assertPosition("Turning 50 round left", sensor2, simul2);

    // Reset index
    indexSimul(simul1, periodInUs);
    assertCount("Reseting index", sensor1, simul1);
    indexSimul(simul2, periodInUs);
    assertCount("Reseting index", sensor2, simul2);

    // Turn 20 round the other side
    moveBothSimulators(false, sensor1.maxPosition * 10 + 42, true, sensor2.maxPosition * 10 + 21, periodInUs);
    assertCount("Turning 10 round + 42 steps left", sensor1, simul1);
    assertCount("Turning 10 round + 21 steps right", sensor2, simul2);

    Serial.println("Simulation finished.");
}
