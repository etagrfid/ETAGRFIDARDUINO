//unix as an ide
//kate
//register for startup bw=rown out/watch dog etc.
//go over decode and make more reliable
//sleep mode
//arduino asleep but leaves rfid on interupt on rfid read?
#include <logger.h>
#include <SPI.h>
#include <SD.h>

logger L;   //pins are set for the board in the library

byte tagData[5]; //Holds the ID numbers from the tag
unsigned long tagID;
const int chipSelect = 10;
File dataFile;// = SD.open("datalog.txt", FILE_WRITE);

clock rtc;
const char daysOfTheWeek[7][12] PROGMEM = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  if (!SD.begin(chipSelect)) {
    const char error[31] PROGMEM = "SD card failed, or not present";
    Serial.println(error);
    return;
  }
  const char greeting[28] PROGMEM = "Please swipe your RFID Tag.";
  Serial.println(greeting);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  dataFile = SD.open("data.csv", FILE_WRITE);
  dataFile.print(':\n');
  dataFile.close();
}

void loop()
{
  if (Serial.available()) {
    L.capture_command();
  }
  
  //scan for a tag - if a tag is sucesfully scanned, return a 'true' and proceed
  if (L.scanForTag(tagData) == true) {
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
    //    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    dataFile = SD.open("data.csv", FILE_WRITE);
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
    delay(L.get_RFID_READ_FREQ());
  }
}// end loop()