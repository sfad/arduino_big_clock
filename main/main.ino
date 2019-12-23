#include <DS3232RTC.h>

#include <Arduino.h>
#include <Wire.h>

#include "myLib.h"

long next_millis = 0;
bool seconds_status = true;
bool seconds_last_status = false;

int last_minute_low = -1;
int last_minute_high = -1;
int last_hour_low = -1;
int last_hour_high = -1;
const uint8_t RTC_1HZ_PIN(3);    // RTC provides a 1Hz interrupt signal on this pin

void setup()
{
    Serial.begin(115200);

    pinMode(RTC_1HZ_PIN, INPUT_PULLUP);     // enable pullup on interrupt pin (RTC SQW pin is open drain)
    attachInterrupt(digitalPinToInterrupt(RTC_1HZ_PIN), incrementTime, FALLING);
    RTC.squareWave(SQWAVE_1_HZ);            // 1 Hz square wave

    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");

    Wire.begin();

    // configure port A as output for Expansion 20
    Wire.beginTransmission(0x20);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();

    // configure port A as output for Expansion 20
    Wire.beginTransmission(0x21);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();

    // configure port A as output for Expansion 20
    Wire.beginTransmission(0x22);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();

    // configure port A as output for Expansion 20
    Wire.beginTransmission(0x23);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of port A to outputs
    Wire.endTransmission();

    next_millis = millis() + 500; // increment by .5 seconds.
}

void loop()
{
    static time_t tLast;
    time_t t = getUTC();

    int t_sec_low = second(t) % 10;
    int t_sec_high = second(t) / 10;

    int t_min_low = minute(t) % 10;
    int t_min_high = minute(t) / 10;

    int t_hour_low = hour(t) % 10;
    int t_hour_high = hour(t) / 10;
    
    if (t != tLast) {
        tLast = t;
        //printTime(t);
        seconds_status = true;
        seconds_last_status = false;
        next_millis = millis() + 500;
    }

    //Display Minutes low bit   
    if(minute(t) != last_minute_low) {
        last_minute_low = t_min_low;
        writeDigit(4, last_minute_low, false);
    }

    //Display Minutes high bit
    if(minute(t) != last_minute_high) {
        last_minute_high = t_min_high;
        writeDigit(3, last_minute_high, seconds_status);
    }

    //Display Hours low bit   
    if(minute(t) != last_hour_low) {
        last_hour_low = t_hour_low;
        writeDigit(2, last_hour_low, false);
    }

    // Blink the seconds on high bit of minutes
    if(seconds_status != seconds_last_status) {
        writeDigit(2, last_hour_low, seconds_status);
    }

    //Display Hours high bit  
    if(minute(t) != last_hour_high) {
        last_hour_high = t_hour_high;
        writeDigit(1, last_hour_high, false);
    }

    if(millis() > next_millis) {
        next_millis = millis() + 500;
        seconds_status = false;
        seconds_last_status = true;
    }

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
