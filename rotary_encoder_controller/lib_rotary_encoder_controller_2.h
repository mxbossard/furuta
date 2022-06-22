#ifndef lib_rotary_encoder_controller_h
#define lib_rotary_encoder_controller_h

#include <Arduino.h>
#include <inttypes.h>

#include "lib_utils.h"
#include "lib_circular_buffer.h"
#include "lib_model.h"
#include "lib_datagram.h"

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

const static int8_t QUADRATURE_STATES[4] = {0, 1, 3, 2};

/*
    Rotary encoder signals schema.Z

             +---------+         +---------+            1
             |         |         |         |
         B   |         |         |         |
             |         |         |         |
         ----+         +---------+         +---------+  0
                   +---------+         +---------+      1
                   |         |         |         |
         A         |         |         |         |
                   |         |         |         |
         +---------+         +---------+         +----- 0
*/

void IRAM_ATTR registerEvent(volatile AngleSensor* sensor, bool eventA) {
    int64_t usTiming = esp_timer_get_time();

    bool aLevel = gpio_get_level((gpio_num_t) sensor->pinA);
    bool bLevel = gpio_get_level((gpio_num_t) sensor->pinB);

    portENTER_CRITICAL(&mux);
    sensor->eventCount ++;

    if ((eventA && aLevel == bLevel) || (!eventA && aLevel != bLevel)) {
        // Increment counter
        pushCircularBuffer(sensor->timings, usTiming);
        sensor->counter ++;
    } else {
        // Decrement counter
        pushCircularBuffer(sensor->timings, -usTiming);
        sensor->counter --;
    }
    
    if (sensor->quadratureMode) {
        sensor->position = absMod16(sensor->counter, sensor->maxPosition);
    } else {
        sensor->position = absMod16(sensor->counter, sensor->maxPosition * 4) / 4;
    }
    portEXIT_CRITICAL(&mux);
}

int8_t IRAM_ATTR getState(bool aLevel, bool bLevel) {
    int8_t sensorLevels = aLevel << 1 | bLevel;
    return QUADRATURE_STATES[sensorLevels];
}

uint8_t IRAM_ATTR isCwDirection(bool aLevel, bool bLevel, bool eventA) {
    // Return true if turning Clock wise
    return (eventA && aLevel == bLevel) || (!eventA && aLevel != bLevel);
}

void IRAM_ATTR registerSmartEvent(volatile AngleSensor* sensor, bool eventA) {
    // If controller missed events since last registered state, take into account missing steps.
    int64_t usTiming = esp_timer_get_time();

    bool aLevel = gpio_get_level((gpio_num_t) sensor->pinA);
    bool bLevel = gpio_get_level((gpio_num_t) sensor->pinB);

    int8_t newState = getState(aLevel, bLevel);
    bool cwDirection = isCwDirection(aLevel, bLevel, eventA);

    portENTER_CRITICAL(&mux);
    // Calculate increment comparing new state with previous state
    int8_t increment;
    if (sensor->previousState == -1) {
        // Initializing: no previous state yet.
        increment = 1;
    } else if (newState == sensor->previousState) {
        // Missed 3 events
        increment = 4;
    } else if (cwDirection) {
        increment = (int8_t) absMod8(newState - sensor->previousState, 4);
    } else {
        increment = (int8_t) absMod8(sensor->previousState - newState, 4);
    }

    // If CCW will decrement counter
    if (!cwDirection) {
        increment *= -1;
    }

    // If missed events use mean time.
    // Negative time for CCW move.
    usTiming /= increment;

    sensor->previousState = newState;
    sensor->eventCount ++;
    pushCircularBuffer(sensor->timings, usTiming);
    sensor->counter += increment;
    sensor->position = absMod16(sensor->counter, sensor->maxPosition);
    if (sensor->quadratureMode) {
        sensor->position = absMod16(sensor->counter, sensor->maxPosition);
    } else {
        sensor->position = absMod16(sensor->counter, sensor->maxPosition * 4) / 4;
    }
    portEXIT_CRITICAL(&mux);    
}

void IRAM_ATTR indexSensor(volatile AngleSensor* sensor) {
    portENTER_CRITICAL(&mux);
    sensor->counter = 0;
    sensor->position = 0;
    sensor->eventCount = 0;
    portEXIT_CRITICAL(&mux);
}

int32_t speedsMessage(char* buf, int64_t* speeds, size_t size) {
    int32_t n = sprintf(buf, "speeds: ");
    n += printArray64as32(&buf[n], speeds, size);
    return n;
}

int32_t timingsMessage(char* buf, int64_t* timings, size_t size) {
    int32_t n = sprintf(buf, "timings: ");
    n += printArray64as32(&buf[n], timings, size);
    return n;
}

const char* sensorMessage(AngleSensor* sensor, int64_t* speeds, size_t speedSize) {
    char* buf = (char*) malloc(sizeof(char) * 128);
    int32_t n = sprintf(buf, "[%s] position: %d ", sensor->name, sensor->position);
    n += speedsMessage(&buf[n], speeds, speedSize);
    return buf;
}

const char* positionMessage(AngleSensor* sensor) {
    char* buf = (char*) malloc(sizeof(char) * 16);
    sprintf(buf, "%d", sensor->position);
    return buf;
}

class RotarySensor {

private:
    volatile CircularBuffer timings;
    uint8_t speedsCount;
    volatile AngleSensor sensor;

    int64_t* timingsBuffer;
    int64_t* speedsBuffer;

public:
    // Constructor
    RotarySensor(uint8_t pinA, uint8_t pinB, uint8_t pinIndex, bool quadratureMode, uint16_t points, uint8_t speedsCount, char* name) :
        timings({}), speedsCount(speedsCount), sensor({name, pinA, pinB, pinIndex, quadratureMode, points, (CircularBuffer*) &timings, 0, 0, 0, -1}) {
            begin();
        }

    void begin() {
        initCircularBuffer((CircularBuffer*) &timings, speedsCount);
        timingsBuffer = (int64_t*) malloc(sizeof(int64_t) * speedsCount);
        speedsBuffer = (int64_t*) malloc(sizeof(int64_t) * speedsCount);

        pinMode(sensor.pinA, INPUT_PULLUP);
        pinMode(sensor.pinB, INPUT_PULLUP);
        pinMode(sensor.pinIndex, INPUT_PULLUP);
    }

    void IRAM_ATTR eventA() {
        registerSmartEvent(&sensor, true);
    }

    void IRAM_ATTR eventB() {
        registerSmartEvent(&sensor, false);
    }

    void IRAM_ATTR eventIndex() {
        indexSensor(&sensor);
    }

    void calculateSpeeds(int64_t now) {
        getDataArrayCircularBuffer((CircularBuffer*) &timings, timingsBuffer, speedsCount);
        for (size_t k = speedsCount - 1 ; k > 0 ; k --) {
            // Calculate instantaneous speed between timings
            if (timingsBuffer[k] > 0) {
                speedsBuffer[k] = abs(timingsBuffer[k - 1]) - timingsBuffer[k];
            } else {
                speedsBuffer[k] = - abs(timingsBuffer[k - 1]) - timingsBuffer[k];
            }
        }

        // Last speed is calculated with current time
        if (timingsBuffer[0] >= 0) {
            speedsBuffer[0] = now - timingsBuffer[0];
        } else {
            speedsBuffer[0] = - now - timingsBuffer[0];
        }
    }

    AngleSensor* getSensor() {
        return (AngleSensor*) &sensor;
    }

};

#endif
