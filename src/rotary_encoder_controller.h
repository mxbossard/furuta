#include <Arduino.h>
#include <SPI.h>
#include <inttypes.h>

#define HSPI_MISO 19
#define HSPI_MOSI 23
#define HSPI_SCLK 18
#define HSPI_CS   5
#define SPI_FREQUENCY 10000

#define SENSOR_1_PIN_A 36
#define SENSOR_1_PIN_B 37
#define SENSOR_1_PIN_INDEX 38

#define SENSOR_2_PIN_A 39
#define SENSOR_2_PIN_B 34
#define SENSOR_2_PIN_INDEX 35

#define SIMUL_1_PIN_A 25
#define SIMUL_1_PIN_B 26
#define SIMUL_1_PIN_INDEX 27

#define SIMUL_2_PIN_A 14
#define SIMUL_2_PIN_B 12
#define SIMUL_2_PIN_INDEX 13

#define LED_PIN 2

//static const int spiClk = 240000000; // 1 MHz
SPIClass * hspi = NULL;

struct AngleSensor {
  const uint8_t pinA;
  const uint8_t pinB;
  const uint8_t pinIndex;
  const uint16_t maxPosition;
  int32_t position;
  const char* name;
};

struct AngleSensorSimulator {
  const uint8_t pinA;
  const uint8_t pinB;
  const uint8_t pinIndex;
  int32_t position;
  int32_t counter;
  bool enabled;
  const char* name;
};

AngleSensor sensor1 = {SENSOR_1_PIN_A, SENSOR_1_PIN_B, SENSOR_1_PIN_INDEX, 4000, 0, "sensor1"};
AngleSensor sensor2 = {SENSOR_2_PIN_A, SENSOR_2_PIN_B, SENSOR_2_PIN_INDEX, 1000, 0, "sensor2"};

AngleSensorSimulator simul1 = {SIMUL_1_PIN_A, SIMUL_1_PIN_B, SIMUL_1_PIN_INDEX, 0, 0, true, "simula1"};
AngleSensorSimulator simul2 = {SIMUL_2_PIN_A, SIMUL_2_PIN_B, SIMUL_2_PIN_INDEX, 0, 0, true, "simula2"};

int32_t absMod32(int32_t a, uint16_t b) {
    int32_t c = a % b;
    return (c < 0) ? c + b : c;
}

void IRAM_ATTR moveSensor1A() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    if (aLevel == bLevel) {
        // Increment position
        sensor1.position ++;
    } else {
        // Decrement position
        sensor1.position --;
    }
    //sensor1.position = absMod32(sensor1.position, sensor1.maxPosition);
}

void IRAM_ATTR moveSensor1B() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    if (aLevel != bLevel) {
        // Increment position
        sensor1.position ++;
    } else {
        // Decrement position
        sensor1.position --;
    }
    //sensor1.position = absMod32(sensor1.position, sensor1.maxPosition);
}

void IRAM_ATTR resetSensor1() {
    sensor1.position = 0;
}

void IRAM_ATTR moveSensor2A() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    if (aLevel == bLevel) {
        // Increment position
        sensor2.position ++;
    } else {
        // Decrement position
        sensor2.position --;
    }
    //sensor2.position = absMod32(sensor2.position, sensor2.maxPosition);
}

void IRAM_ATTR moveSensor2B() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    if (aLevel != bLevel) {
        // Increment position
        sensor2.position ++;
    } else {
        // Decrement position
        sensor2.position --;
    }
    //sensor2.position = absMod32(sensor2.position, sensor2.maxPosition);
}

void IRAM_ATTR resetSensor2() {
    sensor2.position = 0;
}


void printSensorInputs() {
    Serial.printf("INPUTS  [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d", sensor1.name, sensor1.pinA, digitalRead(sensor1.pinA), sensor1.pinB, digitalRead(sensor1.pinB), sensor1.pinIndex, digitalRead(sensor1.pinIndex));
    Serial.printf(" ; [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d\n", sensor2.name, sensor2.pinA, digitalRead(sensor2.pinA), sensor2.pinB, digitalRead(sensor2.pinB), sensor2.pinIndex, digitalRead(sensor2.pinIndex));
}


void printSensor(AngleSensor sensor) {
    delay(1);
    Serial.printf("[%s] position: %d\n", sensor.name, sensor.position);
}

void printSensors() {
    delay(1);
    Serial.printf("[%s] position: %d ; [%s] position: %d\n", sensor1.name, sensor1.position, sensor2.name, sensor2.position);
}

void printSimulatorOutputs() {
    Serial.printf("OUTPUTS [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d", simul1.name, simul1.pinA, digitalRead(simul1.pinA), simul1.pinB, digitalRead(simul1.pinB), simul1.pinIndex, digitalRead(simul1.pinIndex));
    Serial.printf(" ; [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d\n", simul2.name, simul2.pinA, digitalRead(simul2.pinA), simul2.pinB, digitalRead(simul2.pinB), simul2.pinIndex, digitalRead(simul2.pinIndex));
}

void printSimulator(AngleSensorSimulator simulator) {
    delay(1);
    Serial.printf("[%s] position: %d\n", simulator.name, simulator.position);
}

void printSimulators() {
    delay(1);
    Serial.printf("[%s] position: %d ; [%s] position: %d\n", simul1.name, simul1.position, simul2.name, simul2.position);
}

// 4 states
void moveSimulator(AngleSensorSimulator &simulator, bool direction, uint32_t step, uint32_t periodInUs) {
    if (!simulator.enabled) {
        return;
    }
    //Serial.printf("[%s] Moving %d step in direction: %d with period of %d µs\n", simulator.name, step, direction, periodInUs);

    for(uint32_t k = 0; k < step; k++) {
        
        if (direction) {
            simulator.position ++;
            simulator.counter ++;
        } else {
            simulator.position --;
            simulator.counter --;
        }
        
        int32_t state = absMod32(simulator.counter, 4);
        //Serial.printf("simulator new state: %d\n", state);

        switch(state) {
            case 0:
                digitalWrite(simulator.pinA, LOW);
                digitalWrite(simulator.pinB, LOW);
                break;
            case 1:
                digitalWrite(simulator.pinA, LOW);
                digitalWrite(simulator.pinB, HIGH);
                break;
            case 2:
                digitalWrite(simulator.pinA, HIGH);
                digitalWrite(simulator.pinB, HIGH);
                break;
            case 3:
                digitalWrite(simulator.pinA, HIGH);
                digitalWrite(simulator.pinB, LOW);
                break;
            default:
                // should not exists
                Serial.printf("Bad simulator state: %d !!!\n", state);
        }
        delayMicroseconds(periodInUs);

        //printSensorInputs();
        //printSimulatorOutputs();
   }
}

void moveBothSimulators(bool direction1, uint32_t step1, bool direction2, uint32_t step2, uint32_t periodInUs) {
    uint32_t maxStep = max(step1, step2);
    uint32_t minStep = min(step1, step2);
    uint32_t moves1 = 0;
    uint32_t moves2 = 0;
    for (; maxStep > 0; maxStep --) {
        if (minStep > 0) {
            // Do not wait period here
            if (step1 <= step2) {
                moveSimulator(simul1, direction1, 1, 0);
                moves1 ++;
            } else {
                moveSimulator(simul2, direction2, 1, 0);
                moves2 ++;
            }
            minStep --;
        }

        if (step1 > step2) {
            moveSimulator(simul1, direction1, 1, 0);
            moves1 ++;
        } else {
            moveSimulator(simul2, direction2, 1, 0);
            moves2 ++;
        }

        delayMicroseconds(periodInUs);
    }
    // Serial.printf("Moved simul1: %d ; simul2: %d\n", moves1, moves2);
}

void indexSimul(AngleSensorSimulator &simulator) {
    //Serial.printf("[%s] Reseting index\n", simulator.name);
    digitalWrite(simulator.pinIndex, HIGH);
    delay(1);
    digitalWrite(simulator.pinIndex, LOW);
    simulator.position = 0;
}

void assertCount(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return;
    }
    delay(10);
    int32_t expectedCount = simulator.position;
    int32_t drift = abs(expectedCount - sensor.position);

    if (sensor.position != expectedCount) {
        Serial.printf("Bad position for [%s] (%d step drift) expected %d but got %d \"%s\" !\n", sensor.name, drift, expectedCount, sensor.position, message);
        delay(pause);
    }
}

void assertPosition(const char* message, AngleSensor sensor, AngleSensorSimulator simulator, uint32_t pause = 0) {
    if (!simulator.enabled) {
        return;
    }
    delay(10);
    int32_t position = absMod32(sensor.position, sensor.maxPosition);
    int32_t expectedPosition = absMod32(simulator.position, sensor.maxPosition);
    int32_t drift = abs(expectedPosition - position);
    drift = min(drift, sensor.maxPosition - drift);

    if (position != expectedPosition) {
        Serial.printf("Bad position for [%s] (%d step drift) expected %d but got %d \"%s\" !\n", sensor.name, drift, expectedPosition, position, message);
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



void setup() {
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

void testPendulum(uint16_t amplitude1, uint16_t amplitude2, uint16_t bounces, uint32_t periodInUs) {
    indexSimul(simul1);
    indexSimul(simul2);
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

        indexSimul(simul1);
        indexSimul(simul2);

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

        indexSimul(simul1);
        indexSimul(simul2);
    }
}

void loop() {
    //simul1.enabled = false;
    //simul2.enabled = false;

    delay(100);

    Serial.println("Running simulation ...");
    // testModulo();

    // printSimulators();
    // printSensors();

    uint32_t periodInUs = 2;

    indexSimul(simul1);
    indexSimul(simul2);
    assertCount("Reseting index", sensor1, simul1);
    assertCount("Reseting index", sensor2, simul2);
    // printSimulators();
    // printSensors();

    moveBothSimulators(true, 0, false, 1, periodInUs);
    assertCount("Turning 0 steps", sensor1, simul1);
    assertCount("Turning 1 step left", sensor2, simul2);

    moveBothSimulators(true, 1, false, 0, periodInUs);
    assertCount("Turning 1 steps right", sensor1, simul1);
    assertCount("Turning 0 step", sensor2, simul2);

    moveBothSimulators(true, 2, false, 2, periodInUs);
    assertCount("Turning 2 steps right", sensor1, simul1);
    assertCount("Turning 2 step left", sensor2, simul2);

    moveBothSimulators(true, 5, false, 3, periodInUs);
    assertCount("Turning 5 steps right", sensor1, simul1);
    assertCount("Turning 3 step left", sensor2, simul2);

    moveBothSimulators(true, 11, false, 7, periodInUs);
    assertCount("Turning 11 steps right", sensor1, simul1);
    assertCount("Turning 7 steps left", sensor2, simul2);
    
    moveBothSimulators(true, 101, true, 101, periodInUs);
    assertCount("Turning 101 steps right", sensor1, simul1);
    assertCount("Turning 101 steps right", sensor2, simul2);

    moveBothSimulators(true, 3997, false, 1000, periodInUs);
    assertCount("Turning limit testing", sensor1, simul1, 2000);
    assertCount("Turning timit testing", sensor2, simul2, 2000);
    
    moveBothSimulators(false, 1, true, 1, periodInUs);
    assertCount("Turning limit testing", sensor1, simul1);
    assertCount("Turning timit testing", sensor2, simul2);

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
    indexSimul(simul1);
    indexSimul(simul2);
    assertCount("Reseting index", sensor1, simul1);
    assertCount("Reseting index", sensor2, simul2);

    // Turn 20 round the other side
    moveBothSimulators(false, sensor1.maxPosition * 10 + 42, true, sensor2.maxPosition * 10 + 21, periodInUs);
    assertCount("Turning 10 round + 42 steps left", sensor1, simul1);
    assertCount("Turning 10 round + 21 steps right", sensor2, simul2);

    Serial.println("Simulation finished.");
}
