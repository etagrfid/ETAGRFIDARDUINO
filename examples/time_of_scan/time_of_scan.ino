#include <logger.h>
#include <SPI.h>

logger L;   //pins are set for the board in the library

//const int chipSelect = 10;

byte tagData[5]; //Holds the ID numbers from the tag
unsigned long tagID;

clock rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup() {
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  } else {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //sets clock to system clock at time of compilation
  }
  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
}

void loop() {
  if (L.scanForTag(tagData) == true) {
    /*Serial.print("RFID Tag Data: "); //print a header to the Serial port.
      for (int n = 0; n < 5; n++) {
      Serial.print(tagData[n], HEX);
      }
    */
    Serial.print("\n\rRFID Tag ID: ");
    tagID = ((long)tagData[1] << 24) + ((long)tagData[2] << 16) + ((long)tagData[3] << 8) + tagData[4];
    Serial.println(tagID);
    DateTime now = rtc.now();

    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.year(), DEC);
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
}