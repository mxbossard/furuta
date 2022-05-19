#ifndef utils_h
#define utils_h

#include <Arduino.h>


int32_t absMod32(int32_t a, uint16_t b) {
    int32_t c = a % b;
    return (c < 0) ? c + b : c;
}

void initArray(int32_t* a) {
    size_t length = sizeof(a) / sizeof(a[0]);
    for (int8_t k = 0 ; k < length ; k ++) {
        a[k] = 0;
    }
}

int32_t printArray(char* buf, int32_t a[], int32_t size) {
    // Serial.printf("size: %d ", size);
    String message = "[";
    String data = String(a[0]);
    message.concat(data);
    for (int32_t k = 1; k < size; k ++) {
        message.concat("; ");
        String data = String(a[k]);
        message.concat(data);
    }
    message.concat("]");
    return sprintf(buf, message.c_str());
}

#endif