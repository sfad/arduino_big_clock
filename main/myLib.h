#ifndef _MYLIB_H
#define _MYLIB_H

enum LastBit { 
    last_bit_set,
    last_bit_reset,
    last_bit_ignore
};

enum DisplayDigit {
    Digit_Minutes_Low,
    Digit_Minutes_High,
    Digit_Hours_Low,
    Digit_Hours_High
};

int getDigitAddress(DisplayDigit digit);
int getSegmentHex(int number);

#endif // _MYLIB_H

