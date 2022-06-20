#ifndef lib_model_h
#define lib_model_h

#include <Arduino.h>

struct AngleSensor {
    const uint8_t pinA;
    const uint8_t pinB;
    const uint8_t pinIndex;
    const bool quadratureMode;
    const uint16_t maxPosition;
    int32_t counter; // Position not rounded to maxPosition
    uint16_t position; // Position rounded to maxPosition
    uint32_t eventCount; // Event count received
    int8_t previousState; // One of 4 quadrature state
    const char* name;
};

struct AngleSensorSimulator {
  const uint8_t pinA;
  const uint8_t pinB;
  const uint8_t pinIndex;
  const bool quadratureMode;
  const uint16_t maxPosition;
  int32_t counter; // Position not rounded to maxPosition
  uint16_t position; // Position rounded to maxPosition
  uint32_t eventCount; // Event count sent
  int64_t internalState;
  bool enabled;
  const char* name;
};

#endif