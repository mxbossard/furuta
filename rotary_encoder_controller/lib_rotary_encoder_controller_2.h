#ifndef lib_rotary_encoder_controller_h
#define lib_rotary_encoder_controller_h

#include <Arduino.h>
#include <inttypes.h>

#include "lib_utils.h"
#include "lib_circular_buffer.h"
#include "lib_model.h"
// #include "lib_datagram.h"

namespace rotaryEncoderController {

    // FIXME: move mux into RotarySensor object
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

    const static int8_t QUADRATURE_STATES[4] = {0, 1, 3, 2};

    /*
        Rotary encoder signals schema.

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
            circularBuffer::push(sensor->timings, usTiming);
            sensor->counter ++;
        } else {
            // Decrement counter
            circularBuffer::push(sensor->timings, -usTiming);
            sensor->counter --;
        }
        
        if (sensor->quadratureMode) {
            sensor->position = libutils::absMod16(sensor->counter, sensor->points);
        } else {
            sensor->position = libutils::absMod16(sensor->counter, sensor->points * 4) / 4;
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

    void IRAM_ATTR boxPosition(volatile AngleSensor* sensor) {
        //sensor->position = libutils::absMod16(sensor->counter, sensor->points);
        if (sensor->position > sensor->maxValue) {
            int16_t delta = sensor->position - sensor->maxValue - 1;
            sensor->position = sensor->minValue + delta;
        } else if (sensor->position < sensor->minValue) {
            int16_t delta = sensor->minValue - sensor->position - 1;
            sensor->position = sensor->maxValue - delta;
        }
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
            increment = (int8_t) libutils::absMod8(newState - sensor->previousState, 4);
        } else {
            increment = (int8_t) libutils::absMod8(sensor->previousState - newState, 4);
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
        circularBuffer::push(sensor->timings, usTiming);
        sensor->counter += increment;
        sensor->position += increment;

        boxPosition(sensor);

        if (!sensor->quadratureMode) {
            sensor->position = sensor->position / 4;
        }

        portEXIT_CRITICAL(&mux);    
    }

    void IRAM_ATTR indexSensor(volatile AngleSensor* sensor) {
        portENTER_CRITICAL(&mux);
        if (!sensor->indexed) {
            sensor->counter = sensor->offset;
            sensor->position = sensor->offset;
            sensor->indexed = true;
        } else {
            uint16_t modPosition = libutils::absMod16(sensor->position, sensor->points);
            int16_t delta = ((int16_t) modPosition - sensor->offset) % sensor->points;
            if (delta != 0) {
                // Take min range from borders and multiply it by delta sign
                if (abs(0 - delta) < abs(sensor->points - delta)) {
                    delta = 0 - delta;
                } else {
                    delta = sensor->points - delta;
                }
                // Position misaligned with index signal.
                sensor->counter += delta;
                sensor->position += delta;
            }

            boxPosition(sensor);
        }
        
        sensor->eventCount = 0;
        portEXIT_CRITICAL(&mux);
    }

    uint8_t rotarySensorCounter = 0;
    const char* nextRotarySensorName() {
        String name = "sensor";
        name.concat(String(rotarySensorCounter, DEC));
        rotarySensorCounter ++;
        char* buffer = new char[10]; // Allocate memory from the heap
        name.toCharArray(buffer, 10);
        return buffer;
    }
}


class RotarySensor {

private:
    volatile CircularBuffer timings;
    //uint8_t speedsCount;
    volatile AngleSensor sensor;

    int64_t* timingsBuffer;
    int64_t* speedsBuffer;

    void calculateSpeeds(int64_t now) {
        for (size_t k = timings.size - 1 ; k > 0 ; k --) {
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

public:
    // Constructor 1
    RotarySensor(uint8_t pinA, uint8_t pinB, uint8_t pinIndex, bool quadratureMode, uint16_t points, size_t speedsCount) :
        timings({speedsCount}), sensor({rotaryEncoderController::nextRotarySensorName(), pinA, pinB, pinIndex, 
            quadratureMode, points, 0, (int16_t) (points - 1), (CircularBuffer*) &timings, 0, 0, 0, -1, false, 0, 0}) {
            begin();
        }

    // Constructor 2
    RotarySensor(uint8_t pinA, uint8_t pinB, uint8_t pinIndex, bool quadratureMode, uint16_t points, size_t speedsCount, int16_t minValue, uint8_t rounds) :
        timings({speedsCount}), sensor({rotaryEncoderController::nextRotarySensorName(), pinA, pinB, pinIndex, 
            quadratureMode, points, minValue, (int16_t) (minValue + ((int16_t) points) * ((int16_t)rounds) - 1), (CircularBuffer*) &timings, 0, 0, 0, -1, false, 0, 0}) {
            begin();
        }

    void begin() {
        portENTER_CRITICAL(&rotaryEncoderController::mux);
        circularBuffer::init((CircularBuffer*) &timings);
        sensor.counter = 0;
        sensor.eventCount = 0;
        sensor.offset = 0;
        sensor.position = 0;
        sensor.indexed = false;
        sensor.previousState = -1;
        portEXIT_CRITICAL(&rotaryEncoderController::mux);

        pinMode(sensor.pinA, INPUT_PULLUP);
        pinMode(sensor.pinB, INPUT_PULLUP);
        pinMode(sensor.pinIndex, INPUT_PULLUP);

        timingsBuffer = (int64_t*) malloc(sizeof(int64_t) * timings.size);
        speedsBuffer = (int64_t*) malloc(sizeof(int64_t) * timings.size);
    }

    void IRAM_ATTR eventA() {
        rotaryEncoderController::registerSmartEvent(&sensor, true);
    }

    void IRAM_ATTR eventB() {
        rotaryEncoderController::registerSmartEvent(&sensor, false);
    }

    void IRAM_ATTR eventIndex() {
        rotaryEncoderController::indexSensor(&sensor);
    }

    AngleSensor* getSensor() {
        return (AngleSensor*) &sensor;
    }

    /**
     * Build data payload at position p in buffer.
     */
    size_t buildPayload(uint8_t* buffer, size_t p, int64_t now) {
        // Sensor Payload :
        // sensor position on 2 bytes
        // Each speed on 2 bytes

        portENTER_CRITICAL(&rotaryEncoderController::mux);
        uint16_t position = sensor.position;
        circularBuffer::copyDataArray((CircularBuffer*) &timings, timingsBuffer, timings.size);
        portEXIT_CRITICAL(&rotaryEncoderController::mux);

        #ifdef LOG_DEBUG
        int64_t endTransaction = esp_timer_get_time();
        #endif

        calculateSpeeds(now);

        #ifdef LOG_DEBUG
        int64_t endSpeedsCalculation = esp_timer_get_time();
        #endif

        // Position 1
        buffer[p] = position >> 8;
        buffer[p+1] = position;
        p += 2;
        
        // Speeds
        for (size_t k = 0; k < timings.size; k++) {
            // Limit speed to int16_t format
            int16_t limitedSpeed = libutils::int64toInt16(speedsBuffer[k]);
            buffer[p] = limitedSpeed >> 8;
            buffer[p+1] = limitedSpeed;
            //Serial.printf("int32: 0x%08X (%d) ; int16: %d ; int16: 0x%02X%02X\n", speeds[k], speeds[k], speed, buffer[p], buffer[p+1]);
            p += 2;
        }


        #ifdef LOG_DEBUG
        uint32_t transactionTime = (uint32_t) (endTransaction - now);
        uint32_t speedsCalculationTime = (uint32_t) (endSpeedsCalculation - endTransaction);
        Serial.printf("[Timings] transaction: %dµs ; speeds calc: %dµs\n", transactionTime, speedsCalculationTime);

        //printSensors(now);
        printTimings(timingsBuffer, timings1.size);
        printSpeeds(speedsBuffer, timings1.size);
        #endif
        
        return p;
    }

};

#endif
