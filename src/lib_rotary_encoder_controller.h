#ifndef lib_rotary_encoder_controller_h
#define lib_rotary_encoder_controller_h

#include <Arduino.h>
#include <inttypes.h>

#include <lib_utils.h>
#include <lib_circular_buffer.h>
#include <lib_model.h>

#define SPEEDS_COUNT 10

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

CircularBuffer timings1;
CircularBuffer timings2;

AngleSensor sensor1 = {SENSOR_1_PIN_A, SENSOR_1_PIN_B, SENSOR_1_PIN_INDEX, 4000, 0, 0, "sensor1"};
AngleSensor sensor2 = {SENSOR_2_PIN_A, SENSOR_2_PIN_B, SENSOR_2_PIN_INDEX, 1000, 0, 0, "sensor2"};

void IRAM_ATTR moveSensor1A() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    int64_t usTiming = esp_timer_get_time();

    portENTER_CRITICAL(&mux);
    if (aLevel == bLevel) {
        // Increment counter
        pushCircularBuffer(&timings1, usTiming);
        sensor1.counter ++;
    } else {
        // Decrement counter
        pushCircularBuffer(&timings1, -usTiming);
        sensor1.counter --;
    }
    sensor1.position = absMod16(sensor1.counter, sensor1.maxPosition);
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR moveSensor1B() {
    bool aLevel = digitalRead(sensor1.pinA);
    bool bLevel = digitalRead(sensor1.pinB);

    portENTER_CRITICAL(&mux);
    if (aLevel != bLevel) {
        // Increment counter
        pushCircularBuffer(&timings1, esp_timer_get_time());
        sensor1.counter ++;
    } else {
        // Decrement counter
        pushCircularBuffer(&timings1, -esp_timer_get_time());
        sensor1.counter --;
    }
    sensor1.position = absMod16(sensor1.counter, sensor1.maxPosition);
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR resetSensor1() {
    portENTER_CRITICAL(&mux);
    sensor1.counter = 0;
    sensor1.position = 0;
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR moveSensor2A() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    portENTER_CRITICAL(&mux);
    if (aLevel == bLevel) {
        // Increment position
        pushCircularBuffer(&timings2, esp_timer_get_time());
        sensor2.counter ++;
    } else {
        // Decrement position
        pushCircularBuffer(&timings2, -esp_timer_get_time());
        sensor2.counter --;
    }
    sensor2.position = absMod16(sensor2.counter, sensor2.maxPosition);
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR moveSensor2B() {
    bool aLevel = digitalRead(sensor2.pinA);
    bool bLevel = digitalRead(sensor2.pinB);

    portENTER_CRITICAL(&mux);
    if (aLevel != bLevel) {
        // Increment position
        pushCircularBuffer(&timings2, esp_timer_get_time());
        sensor2.counter ++;
    } else {
        // Decrement position
        pushCircularBuffer(&timings2, -esp_timer_get_time());
        sensor2.counter --;
    }
    sensor2.position = absMod16(sensor2.counter, sensor2.maxPosition);
    portEXIT_CRITICAL(&mux);
}

void IRAM_ATTR resetSensor2() {
    portENTER_CRITICAL(&mux);
    sensor2.counter = 0;
    sensor2.position = 0;
    portEXIT_CRITICAL(&mux);
}

int64_t* data = (int64_t*) malloc(sizeof(int64_t) * SPEEDS_COUNT);
int64_t* speeds = (int64_t*) malloc(sizeof(int64_t) * SPEEDS_COUNT);
int64_t* calculateSpeeds0(CircularBuffer cb, int64_t now) {
    getDataArrayCircularBuffer(cb, data, cb.size);

    for (size_t k = cb.size - 1 ; k > 0 ; k --) {
        // Calculate instantaneous speed between timings
        if (data[k] > 0) {
            speeds[k] = abs(data[k - 1]) - data[k];
        } else {
            speeds[k] = - abs(data[k - 1]) - data[k];
        }
    }

    // Last speed is calculated with current time
    if (data[0] >= 0) {
        speeds[0] = now - data[0];
    } else {
        speeds[0] = - now - data[0];
    }

    return speeds;
}

int64_t* calculateSpeeds2(int64_t* data, size_t size, int64_t now) {
    for (size_t k = size - 1 ; k > 0 ; k --) {
        // Calculate instantaneous speed between timings
        if (data[k] > 0) {
            speeds[k] = abs(data[k - 1]) - data[k];
        } else {
            speeds[k] = - abs(data[k - 1]) - data[k];
        }
    }

    // Last speed is calculated with current time
    if (data[0] >= 0) {
        speeds[0] = now - data[0];
    } else {
        speeds[0] = - now - data[0];
    }

    return speeds;
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

const char* sensorMessage(AngleSensor sensor, int64_t* speeds, size_t speedSize) {
    char* buf = (char*) malloc(sizeof(char) * 128);
    int32_t n = sprintf(buf, "[%s] position: %d ", sensor.name, sensor.position);
    n += speedsMessage(&buf[n], speeds, speedSize);
    return buf;
}

int32_t sensorsMessage(char* buf, int64_t now) {
    // FIXME: probleme sur les overflow des timings.
    int64_t* speeds1 = calculateSpeeds0(timings1, now);
    int64_t* speeds2 = calculateSpeeds0(timings2, now);

    int32_t n = sprintf(buf, "[%s] position: %d ", sensor1.name, sensor1.position);
    n += speedsMessage(&buf[n], speeds1, timings1.size);
    n += sprintf(&buf[n], " ; [%s] position: %d ", sensor2.name, sensor2.position);
    n += speedsMessage(&buf[n], speeds2, timings2.size);
    return n;
}

/*
void printSensor(AngleSensor sensor, CircularBuffer* timings, int64_t now, bool newLine = true) {
    const char* msg = sensorMessage(sensor, timings);
    Serial.printf("%s", msg);
    if (newLine) {
        Serial.println();
    }
}
*/

char* sensorsMessageBuffer = (char*) malloc(sizeof(char) * 256);
void printSensors(int64_t now) {
    sensorsMessage(sensorsMessageBuffer, now);
    Serial.printf("%s\n", sensorsMessageBuffer);
}

void printSpeeds(int64_t* speeds, size_t size) {
    speedsMessage(sensorsMessageBuffer, speeds, size);
    Serial.printf("%s\n", sensorsMessageBuffer);
}

void printTimings(int64_t* timings, size_t size) {
    timingsMessage(sensorsMessageBuffer, timings, size);
    Serial.printf("%s\n", sensorsMessageBuffer);
}

uint8_t datagramCounter = 0;
int16_t sensorDatagram(AngleSensor sensor, CircularBuffer cb, uint8_t* buffer, int16_t position) {
    // Sensor Datagram :
    // CRC8 on 1 byte
    // Header on 1 byte (Parity, Speed counts, redondancy, ...)
    // sensor position on 2 bytes
    // Each speed on 2 bytes

    int16_t p = position;

    // Room for CRC8
    p += 1;

    // Header
    buffer[p] = datagramCounter & 0x0F; // 4 bits counter
    p += 1;
    
    int64_t now = esp_timer_get_time();

    portENTER_CRITICAL(&mux);

    // Speeds
    getDataArrayCircularBuffer(cb, data, cb.size);

    // Position
    buffer[p] = sensor.position >> 8;
    buffer[p+1] = sensor.position;
    p += 2;

    portEXIT_CRITICAL(&mux);

    int64_t* speeds = calculateSpeeds2(data, cb.size, now);
    printTimings(data, cb.size);
    printSpeeds(speeds, cb.size);

    for (int16_t k = 0; k < cb.size; k++) {
        // Borne speed to int16_t format
        int64_t limitedSpeed = speeds[k];
        if (limitedSpeed < (int64_t)INT32_MIN) {
            limitedSpeed = INT32_MIN;
        } else if (limitedSpeed > (int64_t)INT32_MAX) {
            limitedSpeed = INT32_MAX;
        }
        buffer[p] = limitedSpeed >> 8;
        buffer[p+1] = limitedSpeed;
        //Serial.printf("int32: 0x%08X (%d) ; int16: %d ; int16: 0x%02X%02X\n", speeds[k], speeds[k], speed, buffer[p], buffer[p+1]);
        p += 2;
    }

    // CRC8 at first position
    buffer[position] = crc8(&buffer[position], position - p);

    // Debug
    Serial.printf("sensor [%s] datagram: ", sensor.name);
    for (int16_t k = position; k < p; k++) {
        Serial.printf("%02X ", buffer[k]);
    }
    Serial.println();

    datagramCounter ++;

    return p - position;
}

void decodeDatagram(uint8_t* buffer, uint16_t wordSize) {
    Serial.printf("Decoded Datagram: ");
    uint8_t crc = buffer[0];
    uint8_t header = buffer[1];
    uint16_t position = (buffer[2] << 8) + buffer[3];

    int16_t speed0 = (buffer[4] << 8) + buffer[5];
    int16_t speed1 = (buffer[6] << 8) + buffer[7];
    int16_t speed2 = (buffer[8] << 8) + buffer[9];

    Serial.printf("CRC: %d Header: %d Position: %d Speed0: %d Speed1: %d Speed2: %d\n", crc, header, position, speed0, speed1, speed2);
}

void printSensorInputs() {
    Serial.printf("INPUTS  [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d", sensor1.name, sensor1.pinA, digitalRead(sensor1.pinA), sensor1.pinB, digitalRead(sensor1.pinB), sensor1.pinIndex, digitalRead(sensor1.pinIndex));
    Serial.printf(" ; [%s] pinA(%d): %d pinB(%d): %d pinIndex(%d): %d\n", sensor2.name, sensor2.pinA, digitalRead(sensor2.pinA), sensor2.pinB, digitalRead(sensor2.pinB), sensor2.pinIndex, digitalRead(sensor2.pinIndex));
}

const char* positionMessage(AngleSensor sensor) {
    char* buf = (char*) malloc(sizeof(char) * 16);
    sprintf(buf, "%d", sensor.position);
    return buf;
}

void controllerSetup() {
    initCircularBuffer(&timings1, SPEEDS_COUNT);
    initCircularBuffer(&timings2, SPEEDS_COUNT);

    // Sensor 1
    pinMode(sensor1.pinA, INPUT_PULLUP);
    pinMode(sensor1.pinB, INPUT_PULLUP);
    pinMode(sensor1.pinIndex, INPUT_PULLUP);
    attachInterrupt(sensor1.pinA, moveSensor1A, CHANGE);
    attachInterrupt(sensor1.pinB, moveSensor1B, CHANGE);
    attachInterrupt(sensor1.pinIndex, resetSensor1, RISING);

    // Sensor 2
    pinMode(sensor2.pinA, INPUT_PULLUP);
    pinMode(sensor2.pinB, INPUT_PULLUP);
    pinMode(sensor2.pinIndex, INPUT_PULLUP);
    attachInterrupt(sensor2.pinA, moveSensor2A, CHANGE);
    attachInterrupt(sensor2.pinB, moveSensor2B, CHANGE);
    attachInterrupt(sensor2.pinIndex, resetSensor2, RISING);
}

#endif