

void setSegmentDigit(int digitNumber, int data, bool lastBit) {
    int digitIndex = digitNumber - 1;
    int addresses[4] = {0x20,0x21,0x22,0x23}; // default address
    int segmentDigit[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
    if(digitNumber > sizeof(addresses)) {
        return;
    }
    if(data > sizeof(segmentDigit)) {
        return;
    }

    int digit = segmentDigit[data];

    if(lastBit) {
        digit = digit | 0X80;
    } else {
        digit = digit & 0x7F;
    }

    Wire.beginTransmission(addresses[digitIndex]);
    Wire.write(0x12); // address port A
    Wire.write(digit); // value to send
    Wire.endTransmission();
}