#include <Arduino.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <DS3232RTC.h>

#include "clockLib.h"
#include "DHT.h"

#define DHTPIN 5        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

volatile time_t isrUTC;         // ISR's copy of current time in UTC

String bt__data = "";
String bt__cmd = "";

DigitalClock clock;
long next_millis = 0;
bool seconds_status = true;
bool seconds_last_status = false;

OperationMode clock__Mode = CLOCK_OP_MODE_CLOCK;
ClockMode display__Mode = CLOCK_MODE_TIME;
bool display__mode_time = false; // when change from other mode reset last states.

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
            //found ; end of data and start data processing
            clock__Mode = btDataProcess();
            bt__data = "";
        }
    }

    switch (clock__Mode)
    {
        case CLOCK_OP_MODE_SCORE:
            /* code */
            break;
        case CLOCK_OP_MODE_TIMER:
            /* code */
            break;
        default: // Clock Mode
            Do_ClockMode();
            break;
    }

    //printTemperature();
}

OperationMode btDataProcess() {
    char input[22];
    bt__data.toCharArray(input, bt__data.length() + 1);
    char *token = strtok(input, " ");

    bt__cmd = token;
    token = strtok(NULL, " ");

    if (bt__cmd == "setTime")
    {
        int bt__time[6] = {0};
        for (int i = 0; i < 5; i++)
        {
            String __out = token;
            bt__time[i] = __out.toInt();
            token = strtok(NULL, " ");
        }

        setTime(
            bt__time[0], // hours
            bt__time[1], // minutes
            bt__time[2], // seconds
            bt__time[3], // day
            bt__time[4], // month
            bt__time[5]  // year
        );
        RTC.set(now()); //set the RTC from the system time
        setUTC(RTC.get());

        Serial.println(bt__time[0]);
        Serial.println(bt__time[1]);
        Serial.println(bt__time[2]);
        Serial.println("you are in [set time mode]");
    }
    
    else if(bt__cmd == "scoreMode") {
        String __team1 = token;
        token = strtok(NULL, " ");
        String __team2 = token;
        Serial.print(F("Team1: "));
        Serial.print(__team1);
        Serial.print(F(" Team2: "));
        Serial.println(__team2);
    }

    // Serial.println(bt__data);
    // Serial.println(bt__cmd);

    return clock.getOperationMode(bt__cmd);
}

// ------  RTC ---------

// return current time
time_t getUTC() {
    noInterrupts();
    time_t utc = isrUTC;
    interrupts();
    return utc;
}

// set the current time
void setUTC(time_t utc) {
    noInterrupts();
    isrUTC = utc;
    interrupts();
}

// 1Hz RTC interrupt handler increments the current time
void incrementTime() {
    ++isrUTC;
}

// format and print a time_t value
void printTime(time_t t) {
    char buf[25];
    char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
    strcpy(m, monthShortStr(month(t)));
    sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d",
        hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t));
    Serial.println(buf);
}

// ------ END RTC ------

void Do_ClockMode() {
    static time_t tLast;
    time_t t = getUTC();
    
    if (t != tLast) {
        tLast = t;
        // printTime(t);
        // printTemperature();
        seconds_status = true;
        seconds_last_status = false;
        next_millis = millis() + 500;

        display__Mode = clock.getClockMode(second(t));
    }

    switch (display__Mode) {
        case CLOCK_MODE_TEMPERATURE:
            Display_Temerature();
            display__mode_time = false;
            break;
        case CLOCK_MODE_HUMIDITY:
            Display_Humidity();
            display__mode_time = false;
            break;
        default:
            // if(!display__mode_time) {
            //     clock.clearDigitsLast();
            // }
            showTimeDigits(t);
            display__mode_time = true;
            break;
    }
    if(millis() > next_millis) {
        next_millis = millis() + 500;
        seconds_status = false;
        seconds_last_status = true;
    }
}

void showTimeDigits(time_t t) {
    // times_current[DIGIT_SECONDS_LOW] = second(t) % 10;
    // times_current[DIGIT_SECONDS_HIGH] = second(t) / 10;

    uint8_t time_minutes_low = minute(t) % 10;
    uint8_t time_minutes_high = minute(t) / 10;

    uint8_t time_hours_low = hourFormat12(t) % 10;
    uint8_t time_hours_high = hourFormat12(t) / 10;

    clock.setDigit(DIGIT_HOURS_LOW, time_hours_low);
    clock.setDigit(DIGIT_HOURS_HIGH, time_hours_high);
    clock.setDigit(DIGIT_MINUTES_LOW, time_minutes_low);
    clock.setDigit(DIGIT_MINUTES_HIGH, time_minutes_high);

    Display_Refresh();
}

void Display_Refresh() {
    //Serial.println("Time Mode");

    clock.displayDigit(DIGIT_HOURS_HIGH);
    clock.displayDigit(DIGIT_HOURS_LOW);
    clock.displayDigit(DIGIT_MINUTES_HIGH);
    clock.displayDigit(DIGIT_MINUTES_LOW);
    // clock.displayDigit(DIGIT_SECONDS_HIGH);
    // clock.displayDigit(DIGIT_SECONDS_LOW);

    // Blink the seconds on low bit of hours
    if(seconds_status != seconds_last_status) {
       clock.changeLastBit(DIGIT_MINUTES_LOW, seconds_status);
    }
}

void Display_Temerature() {
    //Serial.println("Temerature Mode");
    // Read temperature as Celsius (the default)
    float temp = roundf(dht.readTemperature());
    if(isnan(temp)) {
        return;
    }
    uint8_t temp_heigh = (int)temp / 10;
    uint8_t temp_low = (int)temp % 10;
    clock.setDigit(DIGIT_HOURS_HIGH, temp_heigh);
    clock.setDigit(DIGIT_HOURS_LOW, temp_low);
    //show o
    clock.setDigit(DIGIT_MINUTES_HIGH, 19);
    //show C
    clock.setDigit(DIGIT_MINUTES_LOW, 12);

    Display_Refresh();
}

void Display_Humidity() {
    //Serial.println("Humidity Mode");
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = roundf(dht.readHumidity());
    if(isnan(h)) {
        return;
    }
    uint8_t h_heigh = (int)h / 10;
    uint8_t h_low = (int)h % 10;
    clock.setDigit(DIGIT_HOURS_HIGH, h_heigh);
    clock.setDigit(DIGIT_HOURS_LOW, h_low);
    //show h
    clock.setDigit(DIGIT_MINUTES_HIGH, 17);
    //turn off
    clock.setDigit(DIGIT_MINUTES_LOW, -1);

    Display_Refresh();
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
