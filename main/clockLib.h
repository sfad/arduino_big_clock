#ifndef _CLOCK_LIB_H
#define _CLOCK_LIB_H

#define BLUETOOTH_TX 3
#define BLUETOOTH_RX 4

const int DIGIT_HOURS_HIGH = 0;
const int DIGIT_HOURS_LOW = 1;
const int DIGIT_MINUTES_HIGH = 2;
const int DIGIT_MINUTES_LOW = 3;
const int DIGIT_SECONDS_HIGH = 4;
const int DIGIT_SECONDS_LOW = 5;

enum ClockMode {
    CLOCK_MODE_TIME,
    CLOCK_MODE_TEMPERATURE,
    CLOCK_MODE_HUMIDITY
};

class DigitalClock {
    public:
        void begin();
        void setupIOPort(int digit);
        void writeDigit(int digit, int number, bool lastBit);
        int getDigitAddress(int digit);
        int getSegmentHex(int number);
};

#endif // _CLOCK_LIB_H

