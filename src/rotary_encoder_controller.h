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
  uint16_t position;
  char lastEventPin; // not used for now
  const String &name;
};

struct AngleSensorSimulator {
  const uint8_t pinA;
  const uint8_t pinB;
  const uint8_t pinIndex;
  uint8_t state; // 4 states (a, b): (0, 0) (0, 1) (1, 1) (1, 0)
  const String &name;
};

AngleSensor sensor1 = {SENSOR_1_PIN_A, SENSOR_1_PIN_B, SENSOR_1_PIN_INDEX, 4000, 0, 0, String("sensor1")};
AngleSensor sensor2 = {SENSOR_2_PIN_A, SENSOR_2_PIN_B, SENSOR_2_PIN_INDEX, 1000, 0, 0, String("sensor2")};

AngleSensorSimulator simul1 = {SIMUL_1_PIN_A, SIMUL_1_PIN_B, SIMUL_1_PIN_INDEX, 0, String("simul1")};
AngleSensorSimulator simul2 = {SIMUL_2_PIN_A, SIMUL_2_PIN_B, SIMUL_2_PIN_INDEX, 0, String("simul2")};

void IRAM_ATTR moveSensor1A() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    if (aLevel && bLevel || !aLevel && !bLevel) {
        // Increment position
        sensor1.position ++;
        sensor1.position %= sensor1.maxPosition;
    } else {
        // Decrement position
        if (sensor1.position > 0) {
            sensor1.position --;
        } else {
            sensor1.position = sensor1.maxPosition - 1;
        }
    }

    sensor1.lastEventPin = 'a';
}

void IRAM_ATTR moveSensor1B() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    if (!aLevel && bLevel || aLevel && !bLevel) {
        // Increment position
        sensor1.position ++;
        sensor1.position %= sensor1.maxPosition;
    } else {
        // Decrement position
        if (sensor1.position > 0) {
            sensor1.position --;
        } else {
            sensor1.position = sensor1.maxPosition - 1;
        }
    }

    sensor1.lastEventPin = 'b';
}

void IRAM_ATTR resetSensor1() {
    sensor1.position = 0;
}

void IRAM_ATTR moveSensor2A() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    if (aLevel && bLevel || !aLevel && !bLevel) {
        // Increment position
        sensor2.position ++;
        sensor2.position %= sensor2.maxPosition;
    } else {
        // Decrement position
        if (sensor2.position > 0) {
            sensor2.position --;
        } else {
            sensor2.position = sensor2.maxPosition - 1;
        }
    }

    sensor2.lastEventPin = 'a';

    //digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void IRAM_ATTR moveSensor2B() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    if (!aLevel && bLevel || aLevel && !bLevel) {
        // Increment position
        sensor2.position ++;
        sensor2.position %= sensor2.maxPosition;
    } else {
        // Decrement position
        if (sensor2.position > 0) {
            sensor2.position --;
        } else {
            sensor2.position = sensor2.maxPosition - 1;
        }
    }

    sensor2.lastEventPin = 'b';

    //digitalWrite(LED_PIN, !digitalRead(LED_PIN));
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

void moveSimul(AngleSensorSimulator &simulator, bool direction, uint32_t step, uint32_t periodInMs) {
    Serial.printf("[%s] Moving %d step in direction: %d with period of %d ms\n", simulator.name, step, direction, periodInMs);
    for(int k = 0; k < step; k++) {
        //Serial.printf("simulator state: %d\n", simulator.state);
        if (direction) {
            simulator.state ++;
            simulator.state %= 4;
        } else {
            if (simulator.state > 0) {
                simulator.state --;
            } else {
                simulator.state = 3;
            }
        }

        if (simulator.state == 0) {
            digitalWrite(simulator.pinA, LOW);
            digitalWrite(simulator.pinB, LOW);
            // digitalWrite(LED_PIN, LOW);
        } else if (simulator.state == 1) {
            digitalWrite(simulator.pinA, LOW);
            digitalWrite(simulator.pinB, HIGH);
            // digitalWrite(LED_PIN, HIGH);
        } else if (simulator.state == 2) {
            digitalWrite(simulator.pinA, HIGH);
            digitalWrite(simulator.pinB, HIGH);
            // digitalWrite(LED_PIN, LOW);
        } else if (simulator.state == 3) {
            digitalWrite(simulator.pinA, HIGH);
            digitalWrite(simulator.pinB, LOW);
            // digitalWrite(LED_PIN, HIGH);
        } else {
            // should not exists
            Serial.printf("Bad simulator state: %d !!!\n", simulator.state);
        }
        delay(periodInMs);

        //printSensorInputs();
        //printSimulatorOutputs();
   }
}

void indexSimul(AngleSensorSimulator simulator) {
    Serial.printf("[%s] Reseting index\n", simulator.name);
    digitalWrite(simulator.pinIndex, HIGH);
    delay(1);
    digitalWrite(simulator.pinIndex, LOW);
}

void printSensor(AngleSensor sensor) {
    delay(1);
    Serial.printf("[%s] position: %d\n", String(sensor.name), sensor.position);
}

void loop() {
    indexSimul(simul1);
    indexSimul(simul2);

    printSensor(sensor1);
    printSensor(sensor2);

    moveSimul(simul1, true, 3, 2);
    moveSimul(simul2, false, 1, 3);

    printSensor(sensor1);
    printSensor(sensor2);

    moveSimul(simul1, true, 5, 2);
    moveSimul(simul2, false, 3, 3);

    printSensor(sensor1);
    printSensor(sensor2);
    delay(1000);
}
