#include <unity.h>

#include "lib_rotary_encoder_controller_2.h"

void testNewRotarySensor(void) {
  RotarySensor rs = {1, 2, 3, true, 1000, 5};
  rs.begin();
  rs.eventIndex();

  TEST_ASSERT_EQUAL(0, rs.getSensor()->position);
  TEST_ASSERT_EQUAL(1000, rs.getSensor()->points);
  TEST_ASSERT_EQUAL(0, rs.getSensor()->minValue);
  TEST_ASSERT_EQUAL(999, rs.getSensor()->maxValue);
}

void testNewRotarySensorMinValue(void) {
  RotarySensor rs = {1, 2, 3, true, 1000, 5, -250, 1};
  rs.begin();
  rs.eventIndex();

  TEST_ASSERT_EQUAL(0, rs.getSensor()->position);
  TEST_ASSERT_EQUAL(1000, rs.getSensor()->points);
  TEST_ASSERT_EQUAL(-250, rs.getSensor()->minValue);
  TEST_ASSERT_EQUAL(749, rs.getSensor()->maxValue);
}

void testNewRotarySensorMultipleRounds(void) {
  RotarySensor rs = {1, 2, 3, true, 1000, 5, -250, 2};
  rs.begin();
  rs.eventIndex();

  TEST_ASSERT_EQUAL(0, rs.getSensor()->position);
  TEST_ASSERT_EQUAL(1000, rs.getSensor()->points);
  TEST_ASSERT_EQUAL(-250, rs.getSensor()->minValue);
  TEST_ASSERT_EQUAL(1749, rs.getSensor()->maxValue);
}

void testNewRotarySensorOffset(void) {
  RotarySensor rs = {1, 2, 3, true, 1000, 5, -250, 2};
  rs.begin();
  int16_t expectedOffset = 42;
  rs.getSensor()->offset = expectedOffset;
  rs.eventIndex();

  TEST_ASSERT_EQUAL(expectedOffset, rs.getSensor()->position);
  TEST_ASSERT_EQUAL(1000, rs.getSensor()->points);
  TEST_ASSERT_EQUAL(-250, rs.getSensor()->minValue);
  TEST_ASSERT_EQUAL(1749, rs.getSensor()->maxValue);
}

int16_t expectedPosition(boolean quadratureMode, int16_t eventCount, int16_t offset, int16_t minValue, int16_t maxValue) {
  // TODO add break from max to min and min to max.
  int16_t pos = 0;
  if (quadratureMode) {
    pos = offset + 1 + 4 * eventCount;
  } else {
    pos = offset + 1 * eventCount;
  }
  while (pos > maxValue) {
    pos = pos - maxValue - 1 + minValue;
  }
  while (pos < minValue) {
    pos = maxValue - (pos - minValue - 1);
  }
  return pos;
}

void checkSomeMoves(RotarySensor* rs, uint16_t points, int16_t offset, int16_t minValue, int16_t maxValue) {
  bool quadratureMode = rs->getSensor()->quadratureMode;

  rs->begin();
  rs->getSensor()->offset = offset;

  rs->eventIndex();
  TEST_ASSERT_EQUAL_MESSAGE(offset, rs->getSensor()->position, "Bad position after index 1 !");
  TEST_ASSERT_EQUAL_MESSAGE(points, rs->getSensor()->points, "Bad points configured !");
  TEST_ASSERT_EQUAL_MESSAGE(minValue, rs->getSensor()->minValue, "Bad minValue configured !");
  TEST_ASSERT_EQUAL_MESSAGE(maxValue, rs->getSensor()->maxValue, "Bad maxValue configured !");

  // Inputs A and B stay at logical level 0.
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 0, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 1 !");
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 1, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 2 !");
  rs->eventB();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 0, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventB 1 !");
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 1, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 3 !");
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 2, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 4 !");
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 3, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 5 !");
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 4, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 6 !");
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 5, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 7 !");
  rs->eventB();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 4, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventB 2 !");
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 5, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 8 !");
  rs->eventA();
  TEST_ASSERT_EQUAL_MESSAGE(expectedPosition(quadratureMode, 6, offset, minValue, maxValue), rs->getSensor()->position, "Bad position after eventA 9 !");

  rs->eventIndex();
  TEST_ASSERT_EQUAL_MESSAGE(offset, rs->getSensor()->position, "Bad position after index 2 !");
}

void testRotarySensorEventsInSimpleMode(void) {
  // 16 points from 0 to 15
  RotarySensor rs = {1, 2, 3, false, 16, 5};

  checkSomeMoves(&rs, 16, 0, 0, 15);
}

void testRotarySensorEventsInSimpleModeWithOffset(void) {
  // 10 points from 0 to 9
  RotarySensor rs = {1, 2, 3, false, 10, 5};
  int16_t expectedOffset = 7;

  checkSomeMoves(&rs, 10, expectedOffset, 0, 9);
}

void testRotarySensorEventsInSimpleModeOneRound(void) {
  // 10 points from -1 to 8
  RotarySensor rs = {1, 2, 3, false, 10, 5, -1, 1};
  int16_t expectedOffset = 7;

  checkSomeMoves(&rs, 10, expectedOffset, -1, 8);
}

void testRotarySensorEventsInSimpleModeMultipleRounds(void) {
  // 5 points from -2 to 12
  RotarySensor rs = {1, 2, 3, false, 5, 5, -2, 3};
  int16_t expectedOffset = 1;

  checkSomeMoves(&rs, 5, expectedOffset, -2, 12);

  rs.begin();
  rs.getSensor()->offset = expectedOffset;

  rs.eventIndex();
  TEST_ASSERT_EQUAL(expectedOffset, rs.getSensor()->position);
  rs.eventA(); // +0
  rs.eventA(); // +1
  rs.eventA(); // +1
  TEST_ASSERT_EQUAL(expectedOffset + 2, rs.getSensor()->position);
  rs.eventIndex(); // Index must align to offset + N x points
  TEST_ASSERT_EQUAL(expectedOffset, rs.getSensor()->position);
  rs.eventB(); // -1
  rs.eventB(); // -1
  TEST_ASSERT_EQUAL(expectedOffset - 2, rs.getSensor()->position);
  rs.eventIndex(); // Index must align to offset + N x points
  TEST_ASSERT_EQUAL(expectedOffset, rs.getSensor()->position);
  rs.eventB(); // -1
  rs.eventB(); // -1
  rs.eventB(); // -1
  TEST_ASSERT_EQUAL(expectedOffset - 3, rs.getSensor()->position);
  rs.eventIndex(); // Index must align to offset + N x points
  TEST_ASSERT_EQUAL(expectedOffset + 10, rs.getSensor()->position);
  rs.eventB(); // -4
  rs.eventB(); // -4
  TEST_ASSERT_EQUAL(expectedOffset + 8, rs.getSensor()->position);
  rs.eventIndex(); // Index must align to offset + N x points
  TEST_ASSERT_EQUAL(expectedOffset + 10, rs.getSensor()->position);
  
}

void testRotarySensorEventsInQuadratureMode(void) {
  // 16 points from 0 to 15
  RotarySensor rs = {1, 2, 3, true, 16, 5};

  checkSomeMoves(&rs, 16, 0, 0, 15);
}

void testRotarySensorEventsInQuadratureModeWithOffset(void) {
  // 16 points from 0 to 15
  RotarySensor rs = {1, 2, 3, true, 16, 5};
  int16_t expectedOffset = 7;

  checkSomeMoves(&rs, 16, expectedOffset, 0, 15);
}

void testRotarySensorEventsInQuadratureModeOneRound(void) {
  // 16 points from -4 to 11 (1 x 16 -4 -1)
  RotarySensor rs = {1, 2, 3, true, 16, 5, -4, 1};
  int16_t expectedOffset = -2;

  checkSomeMoves(&rs, 16, expectedOffset, -4, 11);
}

void testRotarySensorEventsInQuadratureModeMultipleRounds(void) {
  // 10 points from -4 to 25 (3 x 10 -4 -1)
  RotarySensor rs = {1, 2, 3, true, 10, 5, -4, 3};
  int16_t expectedOffset = 2;

  checkSomeMoves(&rs, 10, expectedOffset, -4, 25);

  rs.begin();
  rs.getSensor()->offset = expectedOffset;

  rs.eventIndex();
  TEST_ASSERT_EQUAL(expectedOffset, rs.getSensor()->position);
  rs.eventA(); // +1
  rs.eventA(); // +4
  rs.eventA(); // +4
  TEST_ASSERT_EQUAL(expectedOffset + 9, rs.getSensor()->position);
  rs.eventIndex(); // Index must align to offset + N x points
  TEST_ASSERT_EQUAL(expectedOffset + 10, rs.getSensor()->position);
  rs.eventB(); // -4
  rs.eventB(); // -4
  TEST_ASSERT_EQUAL(expectedOffset + 2, rs.getSensor()->position);
  rs.eventIndex(); // Index must align to offset + N x points
  TEST_ASSERT_EQUAL(expectedOffset, rs.getSensor()->position);
  rs.eventB(); // -4
  TEST_ASSERT_EQUAL(expectedOffset - 4, rs.getSensor()->position);
  rs.eventIndex(); // Index must align to offset + N x points
  TEST_ASSERT_EQUAL(expectedOffset, rs.getSensor()->position);
  rs.eventB(); // -4
  rs.eventB(); // -4
  TEST_ASSERT_EQUAL(expectedOffset + 22, rs.getSensor()->position);
  rs.eventIndex(); // Index must align to offset + N x points
  TEST_ASSERT_EQUAL(expectedOffset + 20, rs.getSensor()->position);
}

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void setup() {
  UNITY_BEGIN();

  RUN_TEST(testNewRotarySensor);
  RUN_TEST(testNewRotarySensorMinValue);
  RUN_TEST(testNewRotarySensorMultipleRounds);
  RUN_TEST(testNewRotarySensorOffset);

  RUN_TEST(testRotarySensorEventsInSimpleMode);
  RUN_TEST(testRotarySensorEventsInSimpleModeWithOffset);
  RUN_TEST(testRotarySensorEventsInSimpleModeOneRound);
  RUN_TEST(testRotarySensorEventsInSimpleModeMultipleRounds);

  RUN_TEST(testRotarySensorEventsInQuadratureMode);
  RUN_TEST(testRotarySensorEventsInQuadratureModeWithOffset);
  RUN_TEST(testRotarySensorEventsInQuadratureModeOneRound);
  RUN_TEST(testRotarySensorEventsInQuadratureModeMultipleRounds);

  UNITY_END();
}

void loop() {}