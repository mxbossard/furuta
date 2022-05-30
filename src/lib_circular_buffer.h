#ifndef lib_circular_buffer_h
#define lib_circular_buffer_h

#include <Arduino.h>
#include <lib_utils.h>

struct CircularBuffer {
    size_t size;
    int32_t* data; // circular buffer
    int8_t offset;
    int8_t lastPosition;
    int32_t* buffer; // contains ordered data
};

size_t sizeCircularBuffer(CircularBuffer b) {
    return b.size;
}

void pushCircularBuffer(CircularBuffer *b, int32_t data) {
    b->data[b->offset] = data;
    b->offset ++;
    b->lastPosition ++;

    if (b->offset >= b->size) {
        b->offset = 0;
    }
    if (b->lastPosition >= b->size) {
        b->lastPosition = 0;
    }
}

void resetCircularBuffer(CircularBuffer *b) {
    initArray(b->data, b->size);
    initArray(b->buffer, b->size);
    b->offset = 0;
    b->lastPosition = b->size - 1;
}

void initCircularBuffer(CircularBuffer *b, size_t size) {
    b->size = size;
    int32_t* data = (int32_t*) malloc(sizeof(int32_t) * size);
    int32_t* buffer = (int32_t*) malloc(sizeof(int32_t) * size);
    b->data = data;
    b->buffer = buffer;
    resetCircularBuffer(b);
}

int32_t getLastDataCircularBuffer(CircularBuffer b) {
    return b.data[b.lastPosition];
}

int32_t* getDataArrayCircularBuffer(CircularBuffer* b) {
    // Copy data array in new buffer: first item is last pushed
    // Serial.printf("length: %d ", length);
    int32_t* buffer = b->buffer;
    for (int32_t k = 0; k < 5; k ++) {
        int32_t index = b->lastPosition - k;
        if (index < 0) {
            index += b->size;
        }
        buffer[k] = b->data[index];
    }
    // Serial.printf("length: %d ", length);
    return buffer;
}

int32_t printCircularBuffer(char* buf, CircularBuffer* b, bool newLine = true) {
    int32_t n = sprintf(buf, "CircularBuffer: ");
    int32_t* data = getDataArrayCircularBuffer(b);
    int32_t size = sizeCircularBuffer(*b);
    n += printArray(buf, data, size);
    if (newLine) {
        n += sprintf(buf, "\n");
    }
    return n;
}

#endif