#ifndef lib_model_h
#define lib_model_h

#include <Arduino.h>

#include "lib_circular_buffer.h"

struct AngleSensor {
    const char* name;
    const uint8_t pinA;
    const uint8_t pinB;
    const uint8_t pinIndex;
    const bool quadratureMode; // true if quadrature mode enabled : 4 steps by signal period
    //const uint16_t maxPosition; // Position count of sensor
    const uint16_t points; // Sensor point count
    const int16_t minValue; // Minimum value
    const int16_t maxValue; // Maximum value
    CircularBuffer* timings;
    // FIXME: actual counter may overflow if always rotating in same direction without reset.
    int32_t counter; // Position not boxed by min and max values 
    uint16_t position; // Position boxed by min and max values
    uint32_t eventCount; // Event count received
    int8_t previousState; // Last quadrature state received (4 states possible)
    bool indexed; // True if sensor was indexed at least once
    int16_t offset; // Programmable counter offset: index set counter to offset value
    int8_t roundCount; // Number of overflow in positive or negative direction
};

struct AngleSensorSimulator {
  const AngleSensor* sensor;
  const uint8_t pinA;
  const uint8_t pinB;
  const uint8_t pinIndex;
  int32_t counter; // Position not rounded to maxPosition
  uint16_t position; // Position rounded to maxPosition
  uint32_t eventCount; // Event count sent
  int64_t internalState;
  bool enabled;
};

#endif