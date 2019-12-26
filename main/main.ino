#include <DS3232RTC.h>

#include <Arduino.h>
#include <Wire.h>

#include "myLib.h"

volatile time_t isrUTC;         // ISR's copy of current time in UTC

long next_millis = 0;
bool seconds_status = true;
bool seconds_last_status = false;

int times_current[6];
int times_last[6];

// int last_second_low = -1;
// int last_second_high = -1;
// int last_minute_low = -1;
// int last_minute_high = -1;
// int last_hour_low = -1;
// int last_hour_high = -1;

const uint8_t RTC_1HZ_PIN(3);    // RTC provides a 1Hz interrupt signal on this pin

void setup()
{
    Serial.begin(115200);

    pinMode(RTC_1HZ_PIN, INPUT_PULLUP);     // enable pullup on interrupt pin (RTC SQW pin is open drain)
    attachInterrupt(digitalPinToInterrupt(RTC_1HZ_PIN), incrementTime, FALLING);
    RTC.squareWave(SQWAVE_1_HZ);            // 1 Hz square wave

    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus() != timeSet) {
        Serial.println("Unable to sync with the RTC");
    }
    else {
        Serial.println("RTC has set the system time");

        ////setTime(23, 31, 30, 13, 2, 2009);   //set the system time to 23h31m30s on 13Feb2009
        //setTime(13, 02, 00, 24, 12, 2019);
        //RTC.set(now());                     //set the RTC from the system time

        isrUTC = RTC.get();
    }

    Wire.begin();

    setupIOPort(1);
    setupIOPort(2);
    setupIOPort(3);
    setupIOPort(4);

    next_millis = millis() + 500; // increment by .5 seconds.
}

void loop()
{
    static time_t tLast;
    time_t t = getUTC();

    int times_current[DIGIT_SECONDS_LOW] = second(t) % 10;
    int times_current[DIGIT_SECONDS_HIGH] = second(t) / 10;

    int times_current[DIGIT_MINUTES_LOW] = minute(t) % 10;
    int times_current[DIGIT_MINUTES_HIGH] = minute(t) / 10;

    int times_current[DIGIT_HOURS_LOW] = hour(t) % 10;
    int times_current[DIGIT_HOURS_HIGH] = hour(t) / 10;
    
    if (t != tLast) {
        tLast = t;
        //printTime(t);
        seconds_status = true;
        seconds_last_status = false;
        next_millis = millis() + 500;
    }

    // // //Display Seconds low bit   
    // if(t_sec_low != last_second_low) {
    //     last_second_low = t_sec_low;
    //     writeDigit(2, last_second_low, false);
    // }

    // // //Display Seconds high bit   
    // if(t_sec_high != last_second_high) {
    //     last_second_high = t_sec_high;
    //     writeDigit(2, last_second_high, false);
    // }


    // //Display Minutes low bit   
    // if(t_min_low != last_minute_low) {
    //     last_minute_low = t_min_low;
    //     writeDigit(4, last_minute_low, false);
    //     printTime(t);
    // }

    // //Display Minutes high bit
    // if(t_min_high != last_minute_high) {
    //     last_minute_high = t_min_high;
    //     writeDigit(3, last_minute_high, seconds_status);
    // }

    // //Display Hours low bit   
    // if(t_hour_low != last_hour_low) {
    //     last_hour_low = t_hour_low;
    //     writeDigit(2, last_hour_low, false);

    //     //every hour read again the time.
    //     isrUTC = RTC.get();
    //     t = getUTC();
    // }

    // //Display Hours high bit  
    // if(t_hour_high != last_hour_high) {
    //     last_hour_high = t_hour_high;
    //     writeDigit(1, last_hour_high, false);
    // }

    Display_Show(DIGIT_HOURS_HIGH, false);
    Display_Show(DIGIT_HOURS_LOW, false);
    Display_Show(DIGIT_MINUTES_HIGH, seconds_status);
    Display_Show(DIGIT_MINUTES_LOW, false);
    Display_Show(DIGIT_SECONDS_HIGH, false);
    Display_Show(DIGIT_SECONDS_LOW, false);

    // Blink the seconds on high bit of minutes
    if(seconds_status != seconds_last_status) {
       // writeDigit(2, last_hour_low, seconds_status);
       Display_Show(DIGIT_MINUTES_HIGH, seconds_status);
    }

    if(millis() > next_millis) {
        next_millis = millis() + 500;
        seconds_status = false;
        seconds_last_status = true;
    }

}

void Display_Show(int digit, bool lastBit) {
    if(times_current[digit] != times_last[digit]) {
        times_last[digit] = times_current[digit];
        writeDigit(digit, times_current[digit], lastBit);
        if(digit == DIGIT_HOURS_LOW) {
            isrUTC = RTC.get();
        }
    }
}