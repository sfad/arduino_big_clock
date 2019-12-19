#include "myLib.h"

int getDigitAddress(DisplayDigit digit) {
    switch (digit) {
        case Digit_Minutes_High:
            return 0x21;
        case Digit_Hours_Low:
            return 0x22;
        case Digit_Hours_High:
            return 0x23;
        default: // defaut Digit_Minutes_Low
            return 0x20;
    };
}

int getSegmentHex(int number) {
    if(number < 0 || number > 0xFF) {
        return 0;
    }
    int segmentDigit[16] = {
        0x3F, // Hex for digit 0
        0x06, // Hex for digit 1
        0x5B, // Hex for digit 2
        0x4F, // Hex for digit 3
        0x66, // Hex for digit 4
        0x6D, // Hex for digit 5
        0x7D, // Hex for digit 6
        0x07, // Hex for digit 7
        0x7F, // Hex for digit 8
        0x67, // Hex for digit 9
        0x77, // Hex for digit A
        0x7C, // Hex for digit B
        0x39, // Hex for digit C
        0x5E, // Hex for digit D
        0x79, // Hex for digit E
        0x71  // Hex for digit F
    };
    return segmentDigit[number];
}