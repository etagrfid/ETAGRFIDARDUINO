#include <logger.h>
#include <SPI.h>

logger L;   //pins are set for the board in the library

//const int chipSelect = 10;

clock rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup(){
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }else{
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //sets clock to system clock at time of compilation
    }
    Serial.begin(9600);
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
}

void loop(){
    DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    delay(1000);
}