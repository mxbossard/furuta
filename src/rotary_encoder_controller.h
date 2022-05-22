#include <Arduino.h>

#include <lib_rotary_encoder_controller.h>
#include <lib_rotary_encoder_controller_spi_slave.h>

#define LED_PIN 2

void setup() {
    //Serial.begin(115200);

    controllerSetup();
    spiSlaveSetup();

    pinMode(LED_PIN, OUTPUT);
}

int counter = 0;
void loop() {
    spiSlaveProcess();

    // Blink led
    counter ++;
    digitalWrite(LED_PIN, counter % 2);
}