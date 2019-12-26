#ifndef _MYLIB_H
#define _MYLIB_H

const int DIGIT_HOURS_HIGH = 0;
const int DIGIT_HOURS_LOW = 1;
const int DIGIT_MINUTES_HIGH = 2;
const int DIGIT_MINUTES_LOW = 3;
const int DIGIT_SECONDS_HIGH = 4;
const int DIGIT_SECONDS_LOW = 5;

int getDigitAddress(int digit);
int getSegmentHex(int number);

#endif // _MYLIB_H

