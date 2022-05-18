#ifndef rotary_encoder_controller_h
#define rotary_encoder_controller_h

#include <Arduino.h>
#include <SPI.h>
#include <inttypes.h>

#include <utils.h>
#include <circular_buffer.h>

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

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

CircularBuffer timings1 = {0, 0, 0};
CircularBuffer timings2 = {0, 0, 0};

AngleSensor sensor1 = {SENSOR_1_PIN_A, SENSOR_1_PIN_B, SENSOR_1_PIN_INDEX, 4000, 0, "sensor1"};
AngleSensor sensor2 = {SENSOR_2_PIN_A, SENSOR_2_PIN_B, SENSOR_2_PIN_INDEX, 1000, 0, "sensor2"};

void IRAM_ATTR moveSensor1A() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    if (aLevel == bLevel) {
        // Increment position
        portENTER_CRITICAL(&mux);
        sensor1.position ++;
        portEXIT_CRITICAL(&mux);
    } else {
        // Decrement position
        portENTER_CRITICAL(&mux);
        sensor1.position --;
        portEXIT_CRITICAL(&mux);
    }

    // Naive timings: push ms from start at each move
    portENTER_CRITICAL(&mux);
    pushCircularBuffer(&timings1, esp_timer_get_time());
    portEXIT_CRITICAL(&mux);

    //sensor1.position = absMod32(sensor1.position, sensor1.maxPosition);
}

void IRAM_ATTR moveSensor1B() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    if (aLevel != bLevel) {
        // Increment position
        portENTER_CRITICAL(&mux);
        sensor1.position ++;
        portEXIT_CRITICAL(&mux);
    } else {
        // Decrement position
        portENTER_CRITICAL(&mux);
        sensor1.position --;
        portEXIT_CRITICAL(&mux);
    }

    // Naive timings: push ms from start at each move
    portENTER_CRITICAL(&mux);
    pushCircularBuffer(&timings1, esp_timer_get_time());
    portEXIT_CRITICAL(&mux);

    //sensor1.position = absMod32(sensor1.position, sensor1.maxPosition);
}

void IRAM_ATTR resetSensor1() {
    portENTER_CRITICAL(&mux);
    sensor1.position = 0;
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR moveSensor2A() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    if (aLevel == bLevel) {
        // Increment position
        portENTER_CRITICAL(&mux);
        sensor2.position ++;
        portEXIT_CRITICAL(&mux);
    } else {
        // Decrement position
        portENTER_CRITICAL(&mux);
        sensor2.position --;
        portEXIT_CRITICAL(&mux);
    }

    // Naive timings: push ms from start at each move
    portENTER_CRITICAL(&mux);
    pushCircularBuffer(&timings2, esp_timer_get_time());
    portEXIT_CRITICAL(&mux);

    //sensor2.position = absMod32(sensor2.position, sensor2.maxPosition);
}

void IRAM_ATTR moveSensor2B() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    if (aLevel != bLevel) {
        // Increment position
        portENTER_CRITICAL(&mux);
        sensor2.position ++;
        portEXIT_CRITICAL(&mux);
    } else {
        // Decrement position
        portENTER_CRITICAL(&mux);
        sensor2.position --;
        portEXIT_CRITICAL(&mux);
    }

    // Naive timings: push ms from start at each move
    portENTER_CRITICAL(&mux);
    pushCircularBuffer(&timings2, esp_timer_get_time());
    portEXIT_CRITICAL(&mux);

    //sensor2.position = absMod32(sensor2.position, sensor2.maxPosition);
}

void IRAM_ATTR resetSensor2() {
    portENTER_CRITICAL(&mux);
    sensor2.position = 0;
    portEXIT_CRITICAL(&mux);
}


void printSensorInputs() {
    Serial.printf("INPUTS  [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d", sensor1.name, sensor1.pinA, digitalRead(sensor1.pinA), sensor1.pinB, digitalRead(sensor1.pinB), sensor1.pinIndex, digitalRead(sensor1.pinIndex));
    Serial.printf(" ; [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d\n", sensor2.name, sensor2.pinA, digitalRead(sensor2.pinA), sensor2.pinB, digitalRead(sensor2.pinB), sensor2.pinIndex, digitalRead(sensor2.pinIndex));
}


void printSensor(AngleSensor sensor, bool newLine = true) {
    String message = "[%s] position: %d";
    if (newLine) {
        message.concat("\n");
    }
    Serial.printf(message.c_str(), sensor.name, sensor.position);
}

void printSensors() {
    printSensor(sensor1, false);
    Serial.printf(" ");
    printCircularBuffer(timings1, false);
    Serial.printf(" ; ");
    printSensor(sensor2, false);
    Serial.printf(" ");
    printCircularBuffer(timings2, true);
}

#endif