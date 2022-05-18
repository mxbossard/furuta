#include <Arduino.h>
#include <utils.h>

struct CircularBuffer {
  int32_t data[5]; // circular buffer
  int8_t offset;
  int8_t lastPosition;
};

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
    size_t length = sizeof(b.data) / sizeof(b.data[0]);
    //Serial.printf("length: %d\n", length);
    int32_t* dest =(int32_t *) malloc(length*sizeof(int32_t));
    for (int32_t k = 0; k < length; k--) {
        // First item is last one
        int32_t index = b.lastPosition - k;
        if (index < 0) {
            index += length;
        }
        dest[k] = b.data[index];
    }
    return dest;
}

void printCircularBuffer(CircularBuffer b, bool newLine = true) {
    String message = "timings: %s";
    if (newLine) {
        message.concat("\n");
    }
    int32_t* data = getDataArrayCircularBuffer(b);
    const char* timings = printArray(data);
    Serial.printf(message.c_str(), timings);
}