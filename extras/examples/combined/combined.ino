#include <logger.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

// To read RFID correctly on ETAG comment out lines 81 and 82 in C:\Users\Owner\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.6.16\variants\arduino_mzero\variant.h
//#define PIN_LED_RXL          (30u)
//#define PIN_LED_TXL          (31u)

File dataFile;
clock rtc;
byte tagData[5];
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
logger L;
unsigned long tagID;
int chipSelect = 7;//7 => m0; uno => 10

void setup() {
  delay(20000);//delay to allow for reprogramming before serial port gets jammed

  SerialUSB.begin(9600);
  while(!SerialUSB); //needed or 1st print does not work
  SerialUSB.println("Please swipe your RFID Tag.");

  if (!rtc.begin()) {
    SerialUSB.println("Couldn't find RTC!");
  } else {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  L.load_settings();
}

void loop() {
  if (SerialUSB.available()) {
    SerialUSB.println("command received");
    L.capture_command();
  }
  //scan for a tag - if a tag is sucesfully scanned, return a 'true' and proceed
  while(L.scanForTag(tagData) == false){
    SerialUSB.println("No Tag");  
  };
  if (L.scanForTag(tagData) == true) {
    SerialUSB.print("RFID Tag Data: "); //print a header to the Serial port.
    for (int n = 0; n < 5; n++) {
      SerialUSB.print(tagData[n], HEX);
    }
    SerialUSB.print("\n\rRFID Tag ID: ");
    tagID = ((long)tagData[1] << 24) + ((long)tagData[2] << 16) + ((long)tagData[3] << 8) + tagData[4];
    SerialUSB.print(tagID);
    SerialUSB.print("\n\r\n\r");
    DateTime now = rtc.now();
  } else {
    SerialUSB.println("No Tag Detected");
  }

  DateTime now = rtc.now();
  SerialUSB.print(now.year(), DEC);
  SerialUSB.print('/');
  SerialUSB.print(now.month(), DEC);
  SerialUSB.print('/');
  SerialUSB.print(now.day(), DEC);
  SerialUSB.print(" (");
  SerialUSB.print(daysOfTheWeek[now.dayOfTheWeek()]);
  SerialUSB.print(") ");
  SerialUSB.print(now.hour(), DEC);
  SerialUSB.print(':');
  SerialUSB.print(now.minute(), DEC);
  SerialUSB.print(':');
  SerialUSB.print(now.second(), DEC);
  SerialUSB.println();
  delay(200);
}