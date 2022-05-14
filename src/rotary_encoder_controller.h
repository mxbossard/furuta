#include <Arduino.h>
#include <SPI.h>

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
  int64_t position;
  bool enabled;
  const char* name;
};

AngleSensor sensor1 = {SENSOR_1_PIN_A, SENSOR_1_PIN_B, SENSOR_1_PIN_INDEX, 4000, 0, "sensor1"};
AngleSensor sensor2 = {SENSOR_2_PIN_A, SENSOR_2_PIN_B, SENSOR_2_PIN_INDEX, 1000, 0, "sensor2"};

AngleSensorSimulator simul1 = {SIMUL_1_PIN_A, SIMUL_1_PIN_B, SIMUL_1_PIN_INDEX, 0, true, "simul1"};
AngleSensorSimulator simul2 = {SIMUL_2_PIN_A, SIMUL_2_PIN_B, SIMUL_2_PIN_INDEX, 0, true, "simul2"};

int32_t absMod16(int16_t a, uint16_t b) {
    int32_t c = a % b;
    return (c < 0) ? c + b : c;
}

int32_t absMod64(int64_t a, uint16_t b) {
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
    sensor1.position = absMod16(sensor1.position, sensor1.maxPosition);
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
    sensor1.position = absMod16(sensor1.position, sensor1.maxPosition);
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
    sensor2.position = absMod16(sensor2.position, sensor2.maxPosition);
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
    sensor2.position = absMod16(sensor2.position, sensor2.maxPosition);
}

void IRAM_ATTR resetSensor2() {
    sensor2.position = 0;
}


void setup() {
    Serial.begin(115200);
    //hspi = new SPIClass(HSPI);
    //hspi->begin();
    //SPI.begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS); //SCLK, MISO, MOSI, SS
    //pinMode(HSPI_CS, OUTPUT); //HSPI SS
    //digitalWrite(HSPI_CS, HIGH);

    // Sensor 1
    pinMode(sensor1.pinA, INPUT);
    pinMode(sensor1.pinB, INPUT);
    pinMode(sensor1.pinIndex, INPUT);
    attachInterrupt(sensor1.pinA, moveSensor1A, CHANGE);
    attachInterrupt(sensor1.pinB, moveSensor1B, CHANGE);
    attachInterrupt(sensor1.pinIndex, resetSensor1, RISING);

    // Sensor 2
    pinMode(sensor2.pinA, INPUT);
    pinMode(sensor2.pinB, INPUT);
    pinMode(sensor2.pinIndex, INPUT);
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

void printSensorInputs() {
    Serial.printf("INPUTS  [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d", sensor1.name, sensor1.pinA, digitalRead(sensor1.pinA), sensor1.pinB, digitalRead(sensor1.pinB), sensor1.pinIndex, digitalRead(sensor1.pinIndex));
    Serial.printf(" ; [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d\n", sensor2.name, sensor2.pinA, digitalRead(sensor2.pinA), sensor2.pinB, digitalRead(sensor2.pinB), sensor2.pinIndex, digitalRead(sensor2.pinIndex));
}

void printSimulatorOutputs() {
    Serial.printf("OUTPUTS [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d", simul1.name, simul1.pinA, digitalRead(simul1.pinA), simul1.pinB, digitalRead(simul1.pinB), simul1.pinIndex, digitalRead(simul1.pinIndex));
    Serial.printf(" ; [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d\n", simul2.name, simul2.pinA, digitalRead(simul2.pinA), simul2.pinB, digitalRead(simul2.pinB), simul2.pinIndex, digitalRead(simul2.pinIndex));
}

// 4 states
void moveSimulator(AngleSensorSimulator &simulator, bool direction, uint32_t step, uint32_t periodInUs) {
    if (!simulator.enabled) {
        return;
    }

    //Serial.printf("[%s] Moving %d step in direction: %d with period of %d µs\n", simulator.name, step, direction, periodInUs);

    for(int k = 0; k < step; k++) {
        
        if (direction) {
            simulator.position ++;
        } else {
            simulator.position --;
        }
        
        int16_t state = absMod64(simulator.position, 4);
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
    for (; maxStep > 0; maxStep --) {
        if (minStep > 0) {
            // Do not wait period here
            if (step1 <= step2) {
                moveSimulator(simul1, direction1, 1, 0);
            } else {
                moveSimulator(simul2, direction2, 1, 0);
            }
            minStep --;
        }

        if (step1 > step2) {
            moveSimulator(simul1, direction1, 1, 0);
        } else {
            moveSimulator(simul2, direction2, 1, 0);
        }

        delayMicroseconds(periodInUs);
    }
}

void indexSimul(AngleSensorSimulator &simulator) {
    //Serial.printf("[%s] Reseting index\n", simulator.name);
    digitalWrite(simulator.pinIndex, HIGH);
    delay(1);
    digitalWrite(simulator.pinIndex, LOW);
    simulator.position = 0;
}

void printSensor(AngleSensor sensor) {
    delay(1);
    Serial.printf("[%s] position: %d\n", sensor.name, sensor.position);
}

void assertPosition(const char* message, AngleSensor sensor, AngleSensorSimulator simulator) {
    if (!simulator.enabled) {
        return;
    }
    delay(1);
    int16_t expectedPosition = absMod64(simulator.position, sensor.maxPosition);

    if (sensor.position != expectedPosition) {
        Serial.printf("Bad position for [%s] expected %d but got %d \"%s\" !\n", sensor.name, expectedPosition, sensor.position, message);
    }
}

void loop() {
    delay(1000);
    Serial.println("Running simulation ...");

    uint32_t periodInUs = 100;

    indexSimul(simul1);
    indexSimul(simul2);
    assertPosition("Reseting index", sensor1, simul1);
    assertPosition("Reseting index", sensor2, simul2);

    // moveSimul(simul1, true, 3, 2);
    // moveSimul(simul2, false, 1, 3);
    moveBothSimulators(true, 3, false, 1, periodInUs);
    assertPosition("Turning 3 steps right", sensor1, simul1);
    assertPosition("Turning 1 step left", sensor2, simul2);

    // moveSimul(simul1, true, 5, 1);
    // moveSimul(simul2, false, 3, 1);
    moveBothSimulators(true, 5, false, 3, periodInUs);
    assertPosition("Turning 5 steps right", sensor1, simul1);
    assertPosition("Turning 3 steps left", sensor2, simul2);
    
    // moveSimul(simul1, false, 2008, 1);
    // moveSimul(simul2, true, 504, 1);
    moveBothSimulators(false, 2008, true, 504, periodInUs);
    assertPosition("Turning 2008 steps left", sensor1, simul1);
    assertPosition("Turning 504 steps right", sensor2, simul2);

    // Turn 1 round one side
    // moveSimul(simul1, true, sensor1.maxPosition + 3, 1);
    // moveSimul(simul2, false, sensor2.maxPosition - 5, 1);
    moveBothSimulators(true, sensor1.maxPosition + 3, false, sensor2.maxPosition - 5, periodInUs);
    assertPosition("Turning 1 round + 3 steps right", sensor1, simul1);
    assertPosition("Turning 1 round - 5 steps right", sensor2, simul2);
return;
    // Turn 50 round one side
    // moveSimul(simul1, true, sensor1.maxPosition * 50, 1);
    // moveSimul(simul2, false, sensor2.maxPosition * 50, 1);
    moveBothSimulators(true, sensor1.maxPosition * 50, false, sensor2.maxPosition * 50, periodInUs);
    assertPosition("Turning 50 round right", sensor1, simul1);
    assertPosition("Turning 50 round left", sensor2, simul2);

    // Reset index
    indexSimul(simul1);
    indexSimul(simul2);
    assertPosition("Reseting index", sensor1, simul1);
    assertPosition("Reseting index", sensor2, simul2);

    // Turn 20 round the other side
    // moveSimul(simul1, false, sensor1.maxPosition * 20 + 42, 1);
    // moveSimul(simul2, true, sensor2.maxPosition * 20 + 21, 1);
    moveBothSimulators(false, sensor1.maxPosition * 20 + 42, true, sensor2.maxPosition * 20 + 21, periodInUs);
    assertPosition("Turning 20 round + 42 steps left", sensor1, simul1);
    assertPosition("Turning 20 round + 21 steps right", sensor2, simul2);

    Serial.println("Simulation finished.");
}
