#include <DS3232RTC.h>

#include <Arduino.h>
#include <Wire.h>

#include "myLib.h"

long next_millis = 0;
bool seconds_status = false;
bool seconds_last_status = true;

int last_minute_low = 0;
int last_minute_high = 0;
int last_hour_low = 0;
int last_hour_high = 0;
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

    // configure port A as output
    Wire.beginTransmission(0x20);
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
    }

    // if(seconds_status != seconds_last_status) {
    //     setSegmentDigit(1, t_sec_low, seconds_status);
    //     seconds_last_status = seconds_status;
    // }

    //Display Minutes low bit
    LastBit m_lastBit = last_bit_ignore;
    DisplayDigit m_low_digit = Digit_Minutes_Low;    
    if(minute(t) != last_minute_low) {
        last_minute_low = t_min_low;
        writeDigit(m_low_digit, last_minute_low, m_lastBit);
    }

    //Display Minutes high bit
    DisplayDigit m_high_digit = Digit_Minutes_High;
    if(minute(t) != last_minute_high) {
        last_minute_high = t_min_high;
        writeDigit(m_high_digit, last_minute_high, m_lastBit);
    }

    // Blink the seconds on high bit of minutes
    if(seconds_status != seconds_last_status) {
        m_lastBit = seconds_status ? last_bit_set : last_bit_reset;
        seconds_last_status = seconds_status;
        writeDigit(m_high_digit, last_minute_high, m_lastBit);
    }

    //Display Hours low bit
    LastBit h_lastBit = last_bit_ignore;
    DisplayDigit h_low_digit = Digit_Hours_Low;    
    if(minute(t) != last_hour_low) {
        last_hour_low = t_hour_low;
        writeDigit(m_low_digit, last_hour_low, h_lastBit);
    }

    //Display Hours high bit
    DisplayDigit h_high_digit = Digit_Hours_High;    
    if(minute(t) != last_hour_high) {
        last_hour_high = t_hour_high;
        writeDigit(h_high_digit, last_hour_high, h_lastBit);
    }

    if(millis() > next_millis) {
        next_millis = millis() + 500;
        seconds_status = !seconds_status;
    }

}

void writeDigit(DisplayDigit digit, int number, LastBit lastBit) {

    int address = getDigitAddress(digit);
    int digitHex = getSegmentHex(number);

    if(lastBit == last_bit_set) {
        digitHex = digitHex | 0X80;
    } else if(lastBit == last_bit_reset) {
        digitHex = digitHex & 0x7F;
    }

    Wire.beginTransmission(address);
    Wire.write(0x12); // address port A
    Wire.write(digitHex); // value to send
    Wire.endTransmission();
}
