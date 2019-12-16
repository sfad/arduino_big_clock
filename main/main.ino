#include <DS3232RTC.h>

#include <Arduino.h>
#include <Wire.h>

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    // configure port A as output
    Wire.beginTransmission(0x20);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();
}

void loop()
{
    setSegmentDigit(1, 0x20);
}
