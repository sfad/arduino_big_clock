#include <Arduino.h>
#include <Wire.h>
#include "clockLib.h"

void DigitalClock::begin() {
    Wire.begin();

    setupIOPort(DIGIT_HOURS_HIGH, 0x00, 0x00);
    setupIOPort(DIGIT_HOURS_LOW, 0x00, 0x00);
    setupIOPort(DIGIT_MINUTES_HIGH, 0x00, 0x00);
    setupIOPort(DIGIT_MINUTES_LOW, 0x00, 0x00);
    // setupIOPort(DIGIT_SECONDS_HIGH, 0x00, 0x00);
    // setupIOPort(DIGIT_SECONDS_LOW, 0x00, 0x00);
}

int DigitalClock::getDigitAddress(uint8_t digit) {
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

int DigitalClock::getSegmentHex(uint8_t number) {
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

void DigitalClock::setupIOPort(uint8_t digit, uint8_t port, uint8_t portDir) {
    int address = getDigitAddress(digit);
    // configure port A as output for Expansion 20
    Wire.beginTransmission(address);
    Wire.write(port); // 0x00 IO_DIR_A register 
    Wire.write(portDir); // set all of port A to outputs
    Wire.endTransmission();
}

void DigitalClock::setDigit(uint8_t digit, signed char digitValue, bool lastBit) {
    signed char digitHex = getSegmentHex(digitValue);
    if (lastBit) {
        digitHex = digitHex | 0X80;
    }
    else {
        digitHex = digitHex & 0x7F;
    }
    digits_current[digit] = digitHex;
}

void DigitalClock::clearDigitsLast() {
    memset(digits_last, -1, sizeof digits_last);
}

void DigitalClock::writeDigit(uint8_t digit, signed char digitHex) {
    int address = getDigitAddress(digit);
    //Serial.println(address, HEX);

    digits_last[digit] = digitHex;
    //Serial.println(digitHex, HEX);

    Wire.beginTransmission(address);
    Wire.write(0x12); // address port A
    Wire.write(digitHex); // value to send
    Wire.endTransmission();
}

void DigitalClock::displayDigit(uint8_t digit) {
    if(digits_current[digit] != digits_last[digit]) {
        // Serial.print("Digit: ");
        // Serial.print(digit);
        // Serial.print(" Value: ");
        // Serial.print(digits_current[digit]);
        // Serial.print(" Last: ");
        // Serial.println(digits_last[digit]);
        if(digit == DIGIT_MINUTES_HIGH && operationMode == CLOCK_OP_MODE_SCORE) {
            Serial.println("OperationMode: Score");
        }
        signed char _digitHex = digits_current[digit];
        if(digit == DIGIT_HOURS_HIGH || (digit == DIGIT_MINUTES_HIGH && operationMode == CLOCK_OP_MODE_SCORE)) {
            //turn of last digit when it equal to zero
            _digitHex = _digitHex != getSegmentHex(0) ? _digitHex : 0;
            writeDigit(digit, _digitHex);
        } else {
            writeDigit(digit, _digitHex);
        }
    }
}

ClockMode DigitalClock::getClockMode(uint8_t t_seconds) {
    if (t_seconds < 30)
    {
        return CLOCK_MODE_TIME;
    }
    else if (t_seconds < 35)
    {
        return CLOCK_MODE_TEMPERATURE;
    }
    else if (t_seconds < 38)
    {
        return CLOCK_MODE_HUMIDITY;
    }
    else
    {
        return CLOCK_MODE_TIME;
    }
}

void DigitalClock::setOperationMode(String ck__cmd) {
    if (ck__cmd == "scoreMode") {
        operationMode = CLOCK_OP_MODE_SCORE;
    }
    else if (ck__cmd == "timerMode") {
        operationMode =  CLOCK_OP_MODE_TIMER;
    }
    else {
        operationMode =  CLOCK_OP_MODE_CLOCK;
    }
    
    clearDigitsLast();
}

OperationMode DigitalClock::getOperationMode() {
    return operationMode;
}