#include <logger.h>
#include <SPI.h>

logger L;   //pins are set for the board in the library

byte tagData[5]; //Holds the ID numbers from the tag
unsigned long tagID;
clock rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup() {
  SerialUSB.begin(9600);
  while(!SerialUSB);
  if (! rtc.begin()) {
    SerialUSB.println("Couldn't find RTC");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //sets clock to system clock at time of compilation
}

void loop() {
  if (L.scanForTag(tagData) == true) {
    SerialUSB.print("RFID Tag Data: "); //print a header to the SerialUSB port.
    for (int n = 0; n < 5; n++) {
      SerialUSB.print(tagData[n], HEX);
    }
    SerialUSB.print("\n\rRFID Tag ID: ");
    tagID = ((long)tagData[1] << 24) + ((long)tagData[2] << 16) + ((long)tagData[3] << 8) + tagData[4];
    SerialUSB.println(tagID);
    
    DateTime now = rtc.now();
    SerialUSB.print(now.month(), DEC);
    SerialUSB.print('/');
    SerialUSB.print(now.day(), DEC);
    SerialUSB.print('/');
    SerialUSB.print(now.year(), DEC);
    SerialUSB.print(" (");
    SerialUSB.print(daysOfTheWeek[now.dayOfTheWeek()]);
    SerialUSB.print(") ");
    SerialUSB.print(now.hour(), DEC);
    SerialUSB.print(':');
    SerialUSB.print(now.minute(), DEC);
    SerialUSB.print(':');
    SerialUSB.print(now.second(), DEC);
    SerialUSB.println();
    delay(1000);
  }
}