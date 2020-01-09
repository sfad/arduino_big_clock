#include <Wire.h>
#include "clockLib.h"

void DigitalClock::begin() {
    Wire.begin();

    setupIOPort(DIGIT_HOURS_HIGH);
    setupIOPort(DIGIT_HOURS_LOW);
    setupIOPort(DIGIT_MINUTES_HIGH);
    setupIOPort(DIGIT_MINUTES_LOW);
    // setupIOPort(DIGIT_SECONDS_HIGH);
    // setupIOPort(DIGIT_SECONDS_LOW);
}

int DigitalClock::getDigitAddress(int digit) {
    switch (digit) {
        case DIGIT_HOURS_HIGH:
            return 0x20;        
        case DIGIT_HOURS_LOW:
            return 0x21;
        case DIGIT_MINUTES_HIGH:
            return 0x22;
        case DIGIT_MINUTES_LOW:
            return 0x23;
        case DIGIT_SECONDS_HIGH:
            return 0x24;
        case DIGIT_SECONDS_LOW:
            return 0x25;
        default: // defaut Digit_Minutes_Low
            return 0x20;
    };
}

int DigitalClock::getSegmentHex(int number) {
    if(number < 0 || number > 20) {
        return 0;
    }
    int segmentDigit[20] = {
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
        0x71, // Hex for digit F
        0x61, // Hex for digit c
        0x74, // Hex for digit h
        0x76, // Hex for digit H
        0x63  // Hex for digit o
    };
    return segmentDigit[number];
}

void DigitalClock::setupIOPort(int digit) {
    int address = getDigitAddress(digit);
    // configure port A as output for Expansion 20
    Wire.beginTransmission(address);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();
}

void DigitalClock::writeDigit(int digit, int number, bool lastBit) {
    int address = getDigitAddress(digit);
    int digitHex = getSegmentHex(number);
    //Serial.println(address, HEX);

    if(number >= 0) {
        if(lastBit) {
            digitHex = digitHex | 0X80;
        } else{
            digitHex = digitHex & 0x7F;
        }
    }

    //Serial.println(digitHex, HEX);

    Wire.beginTransmission(address);
    Wire.write(0x12); // address port A
    Wire.write(digitHex); // value to send
    Wire.endTransmission();
}
