#include <unity.h>

#include "lib_crc.h"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}


void testCrc8(void) {
  uint8_t message[4] = {0x01, 0x10, 0x23, 0x42};
  uint8_t expected = 0xEC;
  TEST_ASSERT_EQUAL(expected, libcrc::crc8(message, 4));
}

void testSlowXmodemCrc16(void) {
  uint8_t message[4] = {0x01, 0x10, 0x23, 0x42};
  uint16_t expected = 0x0EE4;
  TEST_ASSERT_EQUAL(expected, libcrc::slowXmodemCrc16(message, 4));
}

void testXmodemCrc16(void) {
  uint8_t message[4] = {0x01, 0x10, 0x23, 0x42};
  uint16_t expected = 0x0EE4;
  TEST_ASSERT_EQUAL(expected, libcrc::xmodemCrc16(message, 4));
}

void setup() {
  UNITY_BEGIN();

  RUN_TEST(testCrc8);
  RUN_TEST(testXmodemCrc16);
  RUN_TEST(testSlowXmodemCrc16);

  UNITY_END();
}

void loop() {}
