
void setupIOPort(int digit) {
    int address = getDigitAddress(digit);
    // configure port A as output for Expansion 20
    Wire.beginTransmission(address);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();
}

void writeDigit(int digit, int number, bool lastBit) {
    int address = getDigitAddress(digit);
    int digitHex = getSegmentHex(number);
    //Serial.println(address, HEX);

    if(lastBit) {
        digitHex = digitHex | 0X80;
    } else{
        digitHex = digitHex & 0x7F;
    }

    //Serial.println(digitHex, HEX);

    Wire.beginTransmission(address);
    Wire.write(0x12); // address port A
    Wire.write(digitHex); // value to send
    Wire.endTransmission();
}
