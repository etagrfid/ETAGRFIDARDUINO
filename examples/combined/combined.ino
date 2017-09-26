#include <logger.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

File dataFile;
clock rtc;
byte tagData[5];
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
logger L;
unsigned long tagID;
int chipSelect = 7;//7 on m0 // was 10

void setup() {
  delay(20000);
  SerialUSB.begin(9600);
  // while (!SerialUSB);
  SerialUSB.println("Please swipe your RFID Tag.");

  if (!rtc.begin()) {
    SerialUSB.println("Couldn't find RTC");
  }
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  load_settings();
}

void loop() {
  if (SerialUSB.available()) {
    SerialUSB.println("command received");
    L.capture_command();
  }
  //scan for a tag - if a tag is sucesfully scanned, return a 'true' and proceed
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
  delay(3000);
}

void load_settings() {
  char current_setting[30] = "";
  char value[30] = "";
  int next_index = 0;
  int value_index = 0;
  if (!SD.begin(chipSelect)) {
    SerialUSB.println("Could not find SD card to load settings!");
    return;
  }
  File settings = SD.open("settings.txt");
  if (!settings) {
    // if the file didn't open, print an error:
    SerialUSB.println("Error, SD card detected but could not open settings.txt");
  }
  char letter;
  while (settings.available()) {
    letter = settings.read();
    switch (letter) {
      case '#'://after a comment symbol ignore until new line
        while (settings.available() && letter != '\n' && letter != '\r') {
          letter = settings.read();
        }
        break;
      case '\t':
      case '\n':
      case '\r':
      case ' ': //do nothing with spaces new lines and tabs
        break;
      case ':':
        while (settings.available() && value[value_index - 1] != ';') { //read in setting value
          value[value_index] = settings.read();
          value_index++;
        }

        if (current_setting == "DEFAULT_MODE") { //figure out what the setting is
          SerialUSB.print("setting: default_mode = ");
          SerialUSB.println(value);
        } else if (current_setting == "POWER_SCHEDULE") {
          SerialUSB.print("setting: power schedule = value");
          SerialUSB.println(value);
        } else {
          SerialUSB.print(current_setting);
          SerialUSB.print(" : ");
          SerialUSB.println(value);
        }

        while (value_index > 0) {  //clear value for next use
          value[value_index] = '\0';
          value_index--;
        }

        while (next_index > 0) {  //clear setting for next use
          current_setting[next_index] = '\0';
          next_index--;
        }
        next_index = 0; //reset index
        break;
      default:
        current_setting[next_index] = letter;
        next_index++;
        break;
    }
  }
  settings.close();
}