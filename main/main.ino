#include <Arduino.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <DS3232RTC.h>

#include "scoreBoardLib.h"
#include "DHT.h"

#define DHTPIN 5        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

volatile time_t isrUTC;         // ISR's copy of current time in UTC

String bluetoothData = "";
String bluetoothCommand = "";

ScoreBoard scoreBoard;
long nextMillis = 0;
bool secondsStatus = true;
bool secondsLastStatus = false;

time_t modeTimeout = 0;

ClockMode displayMode = CLOCK_MODE_TIME;

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
        setUTC(RTC.get());
    }

    dht.begin();
    scoreBoard.begin();

    //enable watchdog timer for 1 second
    wdt_enable(WDTO_1S);

    nextMillis = millis() + 500; // increment by .5 seconds.
}

void loop()
{
    //reset the watchdog timer
    wdt_reset();

    time_t t = getUTC();

    char data = 0;
    if(BT.available() > 0) {
        data = BT.read();
        if(data != ';') {
            bluetoothData += data;
        } else {
            //found ; end of data and start data processing
            btDataProcess(t);
            bluetoothData = "";
        }
    }

    switch (scoreBoard.getOperationMode())
    {
        case SCORE_MODE_SCORE:
            // score mode code
            break;
        case SCORE_MODE_TIMER:
            // timer mode code
            break;
        default: // Clock Mode
            modeTimeout = 0;
            DoClockMode();
            break;
    }

    if( t > modeTimeout) {
        scoreBoard.setOperationMode("clockMode");
        modeTimeout = t + 600;
    }
}

//bluetooth data processing.
void btDataProcess(time_t t) {
    char input[22];
    bluetoothData.toCharArray(input, bluetoothData.length() + 1);
    char *token = strtok(input, " ");

    bluetoothCommand = token;
    token = strtok(NULL, " ");

    scoreBoard.setOperationMode(bluetoothCommand);

    if (bluetoothCommand == "setTime")
    {
        int bluetoothTimeData[6] = {0};
        for (int i = 0; i < 5; i++)
        {
            String strToken = token;
            bluetoothTimeData[i] = strToken.toInt();
            token = strtok(NULL, " ");
        }

        setTime(
            bluetoothTimeData[0], // hours
            bluetoothTimeData[1], // minutes
            bluetoothTimeData[2], // seconds
            bluetoothTimeData[3], // day
            bluetoothTimeData[4], // month
            bluetoothTimeData[5]  // year
        );
        RTC.set(now()); //set the RTC from the system time
        setUTC(RTC.get());
    }
    else if(bluetoothCommand == "scoreMode") {
        modeTimeout = t + 600;
        String team1 = token;
        token = strtok(NULL, " ");
        String team2 = token;
        showScoreMode(team1.toInt(), team2.toInt());
    }
    else if(bluetoothCommand == "timerMode") {
        String timerAction = token;

        if(timerAction == "start") {
            //Start timer code

        } else if(timerAction == "pause") {
            //Pause timer code

        } else if(timerAction == "reset") {
            //Reset timer code

        }
    }
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

// ------ END RTC ------

// ---- Clock OperationMode ----

void DoClockMode() {
    static time_t tLast;
    time_t t = getUTC();
    
    if (t != tLast) {
        tLast = t;
        // printTime(t);
        // printTemperature();
        secondsStatus = true;
        secondsLastStatus = false;
        nextMillis = millis() + 500;

        displayMode = scoreBoard.getClockMode(second(t));
    }

    switch (displayMode) {
        case CLOCK_MODE_TEMPERATURE:
            Display_Temerature();
            break;
        case CLOCK_MODE_HUMIDITY:
            Display_Humidity();
            break;
        default:
            showTimeDigits(t);
            break;
    }
    if(millis() > nextMillis) {
        nextMillis = millis() + 500;
        secondsStatus = false;
        secondsLastStatus = true;
    }
}

void showTimeDigits(time_t t) {
    // times_current[DIGIT_SECONDS_LOW] = second(t) % 10;
    // times_current[DIGIT_SECONDS_HIGH] = second(t) / 10;

    uint8_t time_minutes_low = minute(t) % 10;
    uint8_t time_minutes_high = minute(t) / 10;

    uint8_t time_hours_low = hourFormat12(t) % 10;
    uint8_t time_hours_high = hourFormat12(t) / 10;

    scoreBoard.setDigit(DIGIT_HOURS_LOW, time_hours_low, secondsStatus);
    scoreBoard.setDigit(DIGIT_HOURS_HIGH, time_hours_high, false);
    scoreBoard.setDigit(DIGIT_MINUTES_LOW, time_minutes_low, false);
    scoreBoard.setDigit(DIGIT_MINUTES_HIGH, time_minutes_high, false);

    DisplayRefresh();
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
    scoreBoard.setDigit(DIGIT_HOURS_HIGH, temp_heigh, false);
    scoreBoard.setDigit(DIGIT_HOURS_LOW, temp_low, false);
    //show o
    scoreBoard.setDigit(DIGIT_MINUTES_HIGH, 19, false);
    //show C
    scoreBoard.setDigit(DIGIT_MINUTES_LOW, 12, false);

    DisplayRefresh();
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
    scoreBoard.setDigit(DIGIT_HOURS_HIGH, h_heigh, false);
    scoreBoard.setDigit(DIGIT_HOURS_LOW, h_low, false);
    //show h
    scoreBoard.setDigit(DIGIT_MINUTES_HIGH, 17, false);
    //turn off
    scoreBoard.setDigit(DIGIT_MINUTES_LOW, -1, false);

    DisplayRefresh();
}

// ---- END Clock OperationMode ----

// ---- scoreMode ----

void showScoreMode(uint8_t score1, uint8_t score2) {
    uint8_t score1_low = score1 % 10;
    uint8_t score1_high = score1 / 10;

    uint8_t score2_low = score2 % 10;
    uint8_t score2_high = score2 / 10;

    scoreBoard.setDigit(DIGIT_HOURS_LOW, score1_low, false);
    scoreBoard.setDigit(DIGIT_HOURS_HIGH, score1_high, false);
    scoreBoard.setDigit(DIGIT_MINUTES_LOW, score2_low, false);
    scoreBoard.setDigit(DIGIT_MINUTES_HIGH, score2_high, false);

    DisplayRefresh();
}

// ---- END scoreMode ----

void DisplayRefresh() {
    scoreBoard.displayDigit(DIGIT_HOURS_HIGH);
    scoreBoard.displayDigit(DIGIT_HOURS_LOW);
    scoreBoard.displayDigit(DIGIT_MINUTES_HIGH);
    scoreBoard.displayDigit(DIGIT_MINUTES_LOW);
    // scoreBoard.displayDigit(DIGIT_SECONDS_HIGH);
    // scoreBoard.displayDigit(DIGIT_SECONDS_LOW);
}