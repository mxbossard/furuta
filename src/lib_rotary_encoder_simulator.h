#ifndef lib_rotary_encoder_simulator_h
#define lib_rotary_encoder_simulator_h

#include <Arduino.h>

#include <lib_utils.h>
#include <lib_model.h>

AngleSensorSimulator simul1 = {SIMUL_1_PIN_A, SIMUL_1_PIN_B, SIMUL_1_PIN_INDEX, 4000, 0, 0, true, "simula1"};
AngleSensorSimulator simul2 = {SIMUL_2_PIN_A, SIMUL_2_PIN_B, SIMUL_2_PIN_INDEX, 1000, 0, 0, true, "simula2"};

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
void moveSimulator(AngleSensorSimulator* simulator, bool direction, uint32_t step, uint32_t periodInUs) {
    if (!simulator->enabled) {
        return;
    }
    //Serial.printf("[%s] Moving %d step in direction: %d with period of %d µs\n", simulator.name, step, direction, periodInUs);

    for(uint32_t k = 0; k < step; k++) {
        
        if (direction) {
            simulator->position ++;
            simulator->counter ++;
        } else {
            simulator->position --;
            simulator->counter --;
        }
        
        int32_t state = absMod32(simulator->counter, 4);
        //Serial.printf("simulator new state: %d\n", state);

        switch(state) {
            case 0:
                digitalWrite(simulator->pinA, LOW);
                digitalWrite(simulator->pinB, LOW);
                break;
            case 1:
                digitalWrite(simulator->pinA, LOW);
                digitalWrite(simulator->pinB, HIGH);
                break;
            case 2:
                digitalWrite(simulator->pinA, HIGH);
                digitalWrite(simulator->pinB, HIGH);
                break;
            case 3:
                digitalWrite(simulator->pinA, HIGH);
                digitalWrite(simulator->pinB, LOW);
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
                moveSimulator(&simul1, direction1, 1, 0);
                moves1 ++;
            } else {
                moveSimulator(&simul2, direction2, 1, 0);
                moves2 ++;
            }
            minStep --;
        }

        if (step1 > step2) {
            moveSimulator(&simul1, direction1, 1, 0);
            moves1 ++;
        } else {
            moveSimulator(&simul2, direction2, 1, 0);
            moves2 ++;
        }

        delayMicroseconds(periodInUs);
    }
    // Serial.printf("Moved simul1: %d ; simul2: %d\n", moves1, moves2);
}

void indexSimul(AngleSensorSimulator* simulator, uint32_t periodInUs) {
    //Serial.printf("[%s] Reseting index\n", simulator.name);
    digitalWrite(simulator->pinIndex, HIGH);
    delayMicroseconds(periodInUs);
    digitalWrite(simulator->pinIndex, LOW);
    simulator->position = 0;
}

void simulatorSetup() {
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
}

#endif