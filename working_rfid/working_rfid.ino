//unix as an ide
//kate
//register for startup bw=rown out/watch dog etc.
//go over decode and make more reliable
//sleep mode
//arduino asleep but leaves rfid on interupt on rfid read?
//#include <RFIDuino.h> //include the RFIDuino Library
#include <Wire.h>
//#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

RFIDuino myRFIDuino(1.2);   //pins are set for the board in the library

byte tagData[5]; //Holds the ID numbers from the tag
unsigned long tagID;
const int chipSelect = 10;
File dataFile = SD.open("datalog.txt", FILE_WRITE);


RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup()
{
  Serial.begin(9600);
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card failed, or not present");
    return;
  }
  Serial.println("Please swipe your RFID Tag.");
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  String dataString = "test string";
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  /*
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
  */
}

void loop()
{

  //scan for a tag - if a tag is sucesfully scanned, return a 'true' and proceed
  if (myRFIDuino.scanForTag(tagData) == true) {
    Serial.print("RFID Tag Data: "); //print a header to the Serial port.
    for (int n = 0; n < 5; n++) {
      Serial.print(tagData[n], HEX);
    }
    Serial.print("\n\rRFID Tag ID: ");
    tagID = ((long)tagData[1] << 24) + ((long)tagData[2] << 16) + ((long)tagData[3] << 8) + tagData[4];
    Serial.print(tagID);
    Serial.print("\n\r\n\r");
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
    dataFile = SD.open("datalog.txt", FILE_WRITE);
    String dataString = "test string";
    dataFile.println(dataString);
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(" (");
    dataFile.print(daysOfTheWeek[now.dayOfTheWeek()]);
    dataFile.print(") ");
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(':');
    dataFile.print(now.second(), DEC);
    dataFile.println();
    dataFile.close();
    delay(100);
  }
}// end loop()
