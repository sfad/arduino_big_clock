#include <Arduino.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <DS3232RTC.h>

#include "clockLib.h"
#include "DHT.h"

#define DHTPIN 5        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

String bt__data = "";
String bt__cmd = "";

DigitalClock clock;
long next_millis = 0;
bool seconds_status = true;
bool seconds_last_status = false;

ClockMode display__Mode = CLOCK_MODE_TIME;

int times_current[4] = { 0 }; //[6];
int times_last[4] = { -1 }; //[6];

const uint8_t RTC_1HZ_PIN(2);    // RTC provides a 1Hz interrupt signal on this pin
SoftwareSerial BT(BLUETOOTH_RX, BLUETOOTH_TX); //tx, rx

void setup()
{
    Serial.begin(9600);
    BT.begin(9600);
    
    BT.write("AT+NAMEBig_Clock");
    delay(200);
    BT.write("AT+PIN0000");
    delay(200);

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
        // setTime(10, 35, 00, 31, 12, 2019);
        // RTC.set(now());                     //set the RTC from the system time

        setUTC(RTC.get());
    }

    dht.begin();
    clock.begin();

    //enable watchdog timer for 1 second
    wdt_enable(WDTO_1S);

    next_millis = millis() + 500; // increment by .5 seconds.
}

void loop()
{
    //reset the watchdog timer
    wdt_reset();

    char data = 0;
    if(BT.available() > 0) {
        data = BT.read();
        if(data != ';') {
            bt__data += data;
        } else {
            char input[22];
            bt__data.toCharArray(input, bt__data.length() + 1); // "A bird came down the walk";
            char *token = strtok(input, " ");
            int bt__time[6] = { 0 };
            for(int i=0; i < 6; i++) {
                String __out = token;
                if(i>0) {
                    bt__time[i] = __out.toInt();
                } else {
                    bt__cmd = token;
                }
                token = strtok(NULL, " ");
            }

            setTime(
                bt__time[1],    // hours
                bt__time[2],    // minutes
                bt__time[3],    // seconds
                bt__time[4],    // day
                bt__time[5],    // month
                bt__time[6]     // year
            );
            RTC.set(now());                     //set the RTC from the system time
            setUTC(RTC.get());           

            // Serial.println(bt__data);
            // Serial.println(bt__cmd);
            // Serial.println(bt__time[1]);
            // Serial.println(bt__time[2]);
            // Serial.println(bt__time[3]);
            bt__data = "";
        }
    }

    static time_t tLast;
    time_t t = getUTC();
    
    if (t != tLast) {
        tLast = t;
        // printTime(t);
        // printTemperature();
        seconds_status = true;
        seconds_last_status = false;
        next_millis = millis() + 500;

        int __seconds = second(t);
        if(__seconds < 30) {
            display__Mode = CLOCK_MODE_TIME;
        } else if(__seconds < 35 ) {
            display__Mode = CLOCK_MODE_TEMPERATURE;   
        } else if(__seconds < 38) {
            display__Mode = CLOCK_MODE_HUMIDITY;
        } else {
            display__Mode = CLOCK_MODE_TIME;
        }
    }

    switch (display__Mode) {
        case CLOCK_MODE_TEMPERATURE:
            Display_Temerature();
            break;
        case CLOCK_MODE_HUMIDITY:
            Display_Humidity();
            break;
        default:
            //fill array with -1
            memset(times_last, -1, sizeof times_last);
            Display_Time(t);
            break;
    }
    if(millis() > next_millis) {
        next_millis = millis() + 500;
        seconds_status = false;
        seconds_last_status = true;
    }

    //printTemperature();
}

void Display_Time(time_t t ) {
    //Serial.println("Time Mode");
    // times_current[DIGIT_SECONDS_LOW] = second(t) % 10;
    // times_current[DIGIT_SECONDS_HIGH] = second(t) / 10;

    times_current[DIGIT_MINUTES_LOW] = minute(t) % 10;
    times_current[DIGIT_MINUTES_HIGH] = minute(t) / 10;

    times_current[DIGIT_HOURS_LOW] = hourFormat12(t) % 10;
    times_current[DIGIT_HOURS_HIGH] = hourFormat12(t) / 10;

    Display_Show(DIGIT_HOURS_HIGH, false);
    Display_Show(DIGIT_HOURS_LOW, seconds_status);
    Display_Show(DIGIT_MINUTES_HIGH, false);
    Display_Show(DIGIT_MINUTES_LOW, false);
    // Display_Show(DIGIT_SECONDS_HIGH, false);
    // Display_Show(DIGIT_SECONDS_LOW, false);

    // Blink the seconds on high bit of minutes
    if(seconds_status != seconds_last_status) {
       Display_Show(DIGIT_MINUTES_HIGH, seconds_status);
    }
}

void Display_Temerature() {
    //Serial.println("Temerature Mode");
    // Read temperature as Celsius (the default)
    float temp = roundf(dht.readTemperature());
    if(isnan(temp)) {
        return;
    }
    int temp_heigh = (int)temp / 10;
    int temp_low = (int)temp % 10;
    clock.writeDigit(DIGIT_HOURS_HIGH, temp_heigh, false);
    clock.writeDigit(DIGIT_HOURS_LOW, temp_low, false);
    //show o
    clock.writeDigit(DIGIT_MINUTES_HIGH, 19, false);
    //show C
    clock.writeDigit(DIGIT_MINUTES_LOW, 12, false);
}

void Display_Humidity() {
    //Serial.println("Humidity Mode");
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = roundf(dht.readHumidity());
    if(isnan(h)) {
        return;
    }
    int h_heigh = (int)h / 10;
    int h_low = (int)h % 10;
    clock.writeDigit(DIGIT_HOURS_HIGH, h_heigh, false);
    clock.writeDigit(DIGIT_HOURS_LOW, h_low, false);
    //show h
    clock.writeDigit(DIGIT_MINUTES_HIGH, 17, false);
    //turn off
    clock.writeDigit(DIGIT_MINUTES_LOW, -1, false);
}

void Display_Show(int digit, bool lastBit) {
    bool lastBit_changed = false;
    if((digit == DIGIT_HOURS_LOW) && (seconds_status != seconds_last_status)) {
        lastBit_changed = true;
    }

    if((times_current[digit] != times_last[digit]) || lastBit_changed) {
        times_last[digit] = times_current[digit];
        int _digitValue = times_current[digit];
        if(digit == DIGIT_HOURS_HIGH) {
            //trurn of last digit when it equal to zero
            _digitValue = _digitValue > 0 ? _digitValue : -1;
            clock.writeDigit(digit, _digitValue, lastBit);
        } else {
            clock.writeDigit(digit, _digitValue, lastBit);
        }
        if(digit == DIGIT_HOURS_LOW) {
            setUTC(RTC.get());
        }
    }
}

void printTemperature() {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));
}
