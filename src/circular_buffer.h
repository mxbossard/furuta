#include <Arduino.h>
#include <utils.h>

struct CircularBuffer {
  int32_t data[5]; // circular buffer
  int8_t offset;
  int8_t lastPosition;
};

size_t sizeCircularBuffer(CircularBuffer b) {
    size_t size = sizeof(b.data) / sizeof(b.data[0]);
    return size;
}

void pushCircularBuffer(CircularBuffer *b, int32_t data) {
    b->data[b->offset] = data;
    b->offset ++;
    b->lastPosition ++;

    size_t length = sizeof(b->data) / sizeof(b->data[0]);
    if (b->offset >= length) {
        b->offset = 0;
    }
    if (b->lastPosition >= length) {
        b->lastPosition = 0;
    }
}

void resetCircularBuffer(CircularBuffer *b) {
    size_t length = sizeof(b->data) / sizeof(b->data[0]);
    // for (int8_t k = 0 ; k < length; k++) {
    //     pushCircularBuffer(b, 0);
    // }
    initArray(b->data);
    b->offset = 0;
    b->lastPosition = length - 1;
}

int32_t getLastDataCircularBuffer(CircularBuffer b) {
    return b.data[b.lastPosition];
}

int32_t* getDataArrayCircularBuffer(CircularBuffer b) {
    // Copy data array in new buffer: first item is last pushed
    size_t length = sizeof(b.data) / sizeof(b.data[0]);
    // Serial.printf("length: %d ", length);
    int32_t* buf = (int32_t *) malloc(length * sizeof(int32_t));
    for (int32_t k = 0; k < 5; k ++) {
        int32_t index = b.lastPosition - k;
        if (index < 0) {
            index += length;
        }
        buf[k] = b.data[index];
        // buf[k] = k + 1;
    }
    length = sizeof(buf) / sizeof(buf[0]);
    // Serial.printf("length: %d ", length);
    return buf;
}

void printCircularBuffer(CircularBuffer b, bool newLine = true) {
    String message = "timings: %s";
    if (newLine) {
        message.concat("\n");
    }
    int32_t* data = getDataArrayCircularBuffer(b);
    int32_t size = sizeCircularBuffer(b);

    int32_t firstTiming = data[0];
    for (int32_t k = 0 ; k < size ; k ++) {
        data[k] = firstTiming - data[k];
        //data[k] = data[k] - data[size - 1];
    }

    const char* timings = printArray(data, size);
    Serial.printf(message.c_str(), timings);
}