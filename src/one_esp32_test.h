//#include <Arduino.h>

#define LED_PIN 2

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

#include <lib_rotary_encoder_controller.h>
#include <lib_rotary_encoder_simulator.h>
#include <lib_simulator_test.h>

#define LED_PIN 2

void setup() {
    Serial.begin(115200);

    controllerSetup();
    simulatorSetup();

    pinMode(LED_PIN, OUTPUT);

    delay(2000);
}

void loop() {
    //simul1.enabled = false;
    //simul2.enabled = false;

    delay(100);

    Serial.println("Running simulation ...");
    // testModulo();

    // printSimulators();
    // printSensors();

    uint32_t periodInUs = 200;

    bool testFailed = false;

    indexSimul(&simul1, periodInUs);
    testFailed |= !assertData("Reseting index", &sensor1, simul1);
    indexSimul(&simul2, periodInUs);
    testFailed |= !assertData("Reseting index", &sensor2, simul2);
    
    // printSimulators();
    // printSensors();

    moveBothSimulators(true, 0, false, 1, periodInUs);
        
    // printSimulators();
    // printSensors();

    testFailed |= !assertData("Turning 0 steps", &sensor1, simul1);
    testFailed |= !assertData("Turning 1 step left", &sensor2, simul2);

    // printSimulators();
    // printSensors();


    moveBothSimulators(true, 1, false, 0, periodInUs);
    testFailed |= !assertData("Turning 1 steps right", &sensor1, simul1);
    testFailed |= !assertData("Turning 0 step", &sensor2, simul2);

    moveBothSimulators(true, 1, false, 1, periodInUs);
    testFailed |= !assertData("Turning 1 steps right", &sensor1, simul1);
    testFailed |= !assertData("Turning 1 step left", &sensor2, simul2);

    //printSensors();

    moveBothSimulators(false, 2, true, 3, periodInUs);
    testFailed = testFailed || !assertData("Turning 2 steps left", &sensor1, simul1);
    testFailed = testFailed || !assertData("Turning 3 step right", &sensor2, simul2);

    //printSensors();

    moveBothSimulators(true, 3, false, 2, periodInUs);
    testFailed = testFailed || !assertData("Turning 3 steps right", &sensor1, simul1);
    testFailed = testFailed || !assertData("Turning 2 step left", &sensor2, simul2);

    moveBothSimulators(true, 11, false, 7, periodInUs);
    testFailed = testFailed || !assertData("Turning 11 steps right", &sensor1, simul1);
    testFailed = testFailed || !assertData("Turning 7 steps left", &sensor2, simul2);
    
    moveBothSimulators(true, 101, true, 101, periodInUs);
    testFailed = testFailed || !assertData("Turning 101 steps right", &sensor1, simul1);
    testFailed = testFailed || !assertData("Turning 101 steps right", &sensor2, simul2);

    moveBothSimulators(true, 3997, false, 1001, periodInUs);
    testFailed = testFailed || !assertData("Turning 3997 steps right", &sensor1, simul1);
    testFailed = testFailed || !assertData("Turning 1001 steps left", &sensor2, simul2);
   
    // Turn 1 round one side
    moveBothSimulators(true, sensor1.maxPosition + 3, false, sensor2.maxPosition - 5, periodInUs);
    testFailed = testFailed || !assertData("Turning 1 round + 3 steps right", &sensor1, simul1);
    testFailed = testFailed || !assertData("Turning 1 round - 5 steps right", &sensor2, simul2);
    // printSimulators();
    // printSensors();

    testFailed |= !testPendulumWithAssertion(sensor1.maxPosition/2, sensor2.maxPosition/2, 12, periodInUs);

    // Turn 50 round one side
    //moveBothSimulators(true, sensor1.maxPosition * 50, false, sensor2.maxPosition * 50, periodInUs);
    //assertPosition("Turning 50 round right", sensor1, simul1);
    //assertPosition("Turning 50 round left", sensor2, simul2);

    // Reset index
    indexSimul(&simul1, periodInUs);
    testFailed |= !assertData("Reseting index", &sensor1, simul1);
    indexSimul(&simul2, periodInUs);
    testFailed |= !assertData("Reseting index", &sensor2, simul2);

    // Turn 20 round the other side
    moveBothSimulators(false, sensor1.maxPosition * 10 + 42, true, sensor2.maxPosition * 10 + 21, periodInUs);
    testFailed |= !assertData("Turning 10 round + 42 steps left", &sensor1, simul1);
    testFailed |= !assertData("Turning 10 round + 21 steps right", &sensor2, simul2);

    if (testFailed) {
        blinkLed();
    }

    Serial.println("Simulation finished.");
}
