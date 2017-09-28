#include "Arduino.h"
#include <logger.h>  
//
//from RTClib
//
#include <Wire.h>
#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#elif defined(ARDUINO_ARCH_SAMD)
// nothing special needed
#elif defined(ARDUINO_SAM_DUE)
 #define PROGMEM
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
 #define Wire Wire1
#endif



#if (ARDUINO >= 100)
 #include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems
 // Macro to deal with the difference in I2C write functions from old and new Arduino versions.
 #define _I2C_WRITE write
 #define _I2C_READ  read
#else
 #include <WProgram.h>
 #define _I2C_WRITE send
 #define _I2C_READ  receive
#endif
//
//end RTClib
//
#include <SD.h>

const String setting_names[10] PROGMEM = {"BOOT_TO_MODE", "POWER_SCHEDULE", "JSON_RECORD", "CSV_RECORD", "RFID_READ_FREQ", "LOW_BATTERY", "LONGITUDE", "LATITUDE", "FILE_PREFIX", "HOURS_PER_FILE"};

void logger::boot(){
  //load settings
  load_settings();

  //start clock and check it's working
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    //while (1); //comment this out to continue dispite clock not running
  }
}

String logger::time_stamp_string(){
/*  String output = "";
  DateTime now = rtc.now();
  uint32_t time = rtc.unixtime();
  int mask = 1;
  while(mask < time) mask = mask * 10;
  while(mask > 1){
    output += (time / mask) % 10;
  }
  */
  //output += String(rtc.unixtime());
/*  output += String(now.year(), DEC);
  output += '/';
  output += now.month();
  output += '/';
  output += now.day();
  output += ' ';
  output += now.hour();
  output += ':';
  output += now.minute();
  output += ':';
  output += now.second();
  */
  String output = "";
  return output;
}

void logger::capture_command(){
  char letter = 0;
    String setting = "";
    while (true) {//add timeout function
      while (!Serial.available());
      letter = (char)Serial.read();
      if (letter == ':') {
        break;
      } else {
        setting += letter;
      }
    }
    String value = "";
     while (true) {//add timeout function
      while (!Serial.available());
      letter = (char)Serial.read();
      if (letter == ';') {
        break;
      } else {
        value += letter;
      }
    }
    set_setting(setting, value);
}

void logger::set_setting(String setting, String value){
  //get rid of spaces tabs and new lines at the front of setting and value
  while(setting[0] == ' '|| setting[0] == '\t'|| setting[0] == '\n'){
    setting = setting.substring(1);
  }
  while(value[0] == ' '|| value[0] == '\t'|| value[0] == '\n'){
    value = value.substring(1);
  }
  Serial.println(setting);
  Serial.println(value);


  if (setting == setting_names[0]) {  //BOOT_TO_MODE
    BOOT_TO_MODE = value.toInt();
    return;
  } else if (setting == setting_names[1]) { //POWER_SCHEDULE
//needs parsed
    POWER_SCHEDULE = value;
    return;
  } else if (setting == setting_names[2]) { //JSON_RECORD
    if (value == "1") {
      JSON_RECORD = true;
    } else{
      JSON_RECORD = false;
    }
    return;
  } else if (setting == setting_names[3]) { //CSV_RECORD
    if (value == "1") {
      CSV_RECORD = true;
    } else if(value == "0"){
      CSV_RECORD = false;
    }
    return;
  } else if (setting == setting_names[4]) { //RFID_READ_FREQ
    RFID_READ_FREQ = value.toInt();
    return;
  } else if (setting == setting_names[5]) { //LOW_BATTERY
    LOW_BATTERY = value.toInt();
    return;
  } else if (setting == setting_names[6]) { //LONGITUDE
    LONGITUDE = value.toInt();
    return;
  } else if (setting == setting_names[7]) { //LATITUDE
    LATITUDE = value.toInt();
    return;
  } else if (setting == setting_names[8]) { //FILE_PREFIX
    FILE_PREFIX = value;
    return;
  } else if (setting == setting_names[9]) { //HOURS_PER_FILE
    HOURS_PER_FILE = value.toInt();
    return;
  } else {
    Serial.print("could not interpret: -");
    Serial.print(setting);
    Serial.println("-");
    //Serial.println(value);
    Serial.println(setting_names[7]);
    return;
  }
}

void logger::load_settings(){
  char current_setting[30] = "";
  char value[30] = "";
  int next_index = 0;
  int value_index = 0;
  if (!SD.begin(7)){//chipSelect)) {
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
  return;
}


//initialize the RFIDuino object and set the pins correctlly based on hardware version
logger::logger(){
  //RFID related pins
  demodOut = 30;
  shd = 4;
  mod = 2;
  rdyClk = 1;
  //set pin modes on RFID pins
  pinMode(mod, OUTPUT);
  pinMode(shd, OUTPUT);
  pinMode(demodOut, INPUT);
  pinMode(rdyClk, INPUT);

  //set shd and MOD low to prepare for reading
  digitalWrite(shd, LOW);
  digitalWrite(mod, LOW);

  //turn on RFID chip
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

}

//Manchester decode. Supply the function an array to store the tags ID in
bool logger::decodeTag(unsigned char *buf){
  unsigned char i = 0;
  unsigned short timeCount;
  unsigned char timeOutFlag = 0;
  unsigned char row, col;
  unsigned char row_parity;
  unsigned char col_parity[5];
  unsigned char dat;
  unsigned char searchCount = 0;
  unsigned char j;
  while(1){
    timeCount = 0;
    while(0 == digitalRead(demodOut)){//watch for demodOut to go low
      if(timeCount >= TIMEOUT){ //if we pass TIMEOUT milliseconds, break out of the loop
        break;
      } else {
        timeCount++;
      }
    } if (timeCount >= 600) {
      return false;
    }
    timeCount = 0;
    delayMicroseconds(DELAYVAL);
    if(digitalRead(demodOut)) {
      for(i = 0; i < 8; i++) {// 9 header bits
        timeCount = 0; //restart counting
        while(1 == digitalRead(demodOut)) {//while DEMOD out is high
          if(timeCount == TIMEOUT){
            timeOutFlag = 1;
            break;
          } else {
            timeCount++;
          }
        }
        if(timeOutFlag) {
          break;
        } else {
          delayMicroseconds(DELAYVAL);
          if( 0 == digitalRead(demodOut) ){
            break;
          }
        }
      }
      if(timeOutFlag){
        timeOutFlag = 0;
        return false;
      }
      if(i == 8){ //Receive the data
        timeOutFlag = 0;
        timeCount = 0;
        while(1 == digitalRead(demodOut)){
          if(timeCount == TIMEOUT){
            timeOutFlag = 1;
            break;
          } else {
            timeCount++;
          }
          if(timeOutFlag){
            timeOutFlag = 0;
            return false;
          }
        }
        col_parity[0] = col_parity[1] = col_parity[2] = col_parity[3] = col_parity[4] = 0;
        for(row = 0; row < 11; row++){
          row_parity = 0;
          j = row >> 1;
          for(col = 0, row_parity = 0; col < 5; col++){
            delayMicroseconds(DELAYVAL);
            if(digitalRead(demodOut)){
              dat = 1;
            } else {
              dat = 0;
            }
            if(col < 4 && row < 10){
              buf[j] <<= 1;
              buf[j] |= dat;
            }
            row_parity += dat;
            col_parity[col] += dat;
            timeCount = 0;
            while(digitalRead(demodOut) == dat){
              if(timeCount == TIMEOUT){
                timeOutFlag = 1;
                break;
              } else {
                timeCount++;
              }
            }
            if(timeOutFlag) {
              break;
            }
          }
          if(row < 10){
            if((row_parity & 0x01) || timeOutFlag){ //Row parity
              timeOutFlag = 1;
              break;
            }
          }
        }

        if( timeOutFlag || (col_parity[0] & 0x01) || (col_parity[1] & 0x01) || (col_parity[2] & 0x01) || (col_parity[3] & 0x01) ){ //Column parity
          timeOutFlag = 0;
          return false;
        } else {
          return true;
        }
      }//end if(i==8)
      return false;
    }//if(digitalRead(demodOut))
  } //while(1)
};

//function to compare 2 byte arrays. Returns true if the two arrays match, false of any numbers do not match
bool logger::compareTagData(byte *tagData1, byte *tagData2){
  for(int j = 0; j < 5; j++){
    if (tagData1[j] != tagData2[j]){
      return false; //if any of the ID numbers are not the same, return a false
    }
  }
  return true;  //all id numbers have been verified
}

//function to transfer one byte array to a secondary byte array.
//source -> tagData
//destination -> tagDataBuffer
void logger::transferToBuffer(byte *tagData, byte *tagDataBuffer){
  for(int j = 0; j < 5; j++){
    tagDataBuffer[j] = tagData[j];
  }
}

bool logger::scanForTag(byte *tagData){
  static byte tagDataBuffer[5];      //A Buffer for verifying the tag data. 'static' so that the data is maintained the next time the loop is called
  static int readCount = 0;          //the number of times a tag has been read. 'static' so that the data is maintained the next time the loop is called
  boolean verifyRead = false; //true when a tag's ID matches a previous read, false otherwise
  boolean tagCheck = false;   //true when a tag has been read, false otherwise
  tagCheck = decodeTag(tagData); //run the decodetag to check for the tag
  if (tagCheck == true) //if 'true' is returned from the decodetag function, a tag was succesfully scanned
  {
    readCount++;      //increase count since we've seen a tag
    if(readCount == 1) //if have read a tag only one time, proceed
    {
      transferToBuffer(tagData, tagDataBuffer);  //place the data from the current tag read into the buffer for the next read
    }
    else if(readCount == 2) //if we see a tag a second time, proceed
    {
      verifyRead = compareTagData(tagData, tagDataBuffer); //run the checkBuffer function to compare the data in the buffer (the last read) with the data from the current read
      if (verifyRead == true) //if a 'true' is returned by compareTagData, the current read matches the last read
      {
        readCount = 0; //because a tag has been succesfully verified, reset the readCount to '0' for the next tag
        return true;
      }
    }
  }
  else
  {
    return false;
  }
}
//
//CLOCK
//
#include <Wire.h>
#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#elif defined(ARDUINO_ARCH_SAMD)
// nothing special needed
#elif defined(ARDUINO_SAM_DUE)
 #define PROGMEM
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
 #define Wire Wire1
#endif



#if (ARDUINO >= 100)
 #include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems
 // Macro to deal with the difference in I2C write functions from old and new Arduino versions.
 #define _I2C_WRITE write
 #define _I2C_READ  read
#else
 #include <WProgram.h>
 #define _I2C_WRITE send
 #define _I2C_READ  receive
#endif


static uint8_t read_i2c_register(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire._I2C_WRITE((byte)reg);
  Wire.endTransmission();

  Wire.requestFrom(addr, (byte)1);
  return Wire._I2C_READ();
}

static void write_i2c_register(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire._I2C_WRITE((byte)reg);
  Wire._I2C_WRITE((byte)val);
  Wire.endTransmission();
}


////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed

const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

// number of days since 2000/01/01, valid for 2001..2099
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second

DateTime::DateTime (uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ; ++yOff) {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

DateTime::DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    if (year >= 2000)
        year -= 2000;
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

DateTime::DateTime (const DateTime& copy):
  yOff(copy.yOff),
  m(copy.m),
  d(copy.d),
  hh(copy.hh),
  mm(copy.mm),
  ss(copy.ss)
  {}

static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

// A convenient constructor for using "the compiler's time":
//   DateTime now (__DATE__, __TIME__);
// NOTE: using F() would further reduce the RAM footprint, see below.
DateTime::DateTime (const char* date, const char* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    yOff = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
    switch (date[0]) {
        case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

// A convenient constructor for using "the compiler's time":
// This version will save RAM by using PROGMEM to store it by using the F macro.
//   DateTime now (F(__DATE__), F(__TIME__));
DateTime::DateTime (const __FlashStringHelper* date, const __FlashStringHelper* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    char buff[11];
    memcpy_P(buff, date, 11);
    yOff = conv2d(buff + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (buff[0]) {
        case 'J': m = buff[1] == 'a' ? 1 : m = buff[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = buff[2] == 'r' ? 4 : 8; break;
        case 'M': m = buff[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d = conv2d(buff + 4);
    memcpy_P(buff, time, 8);
    hh = conv2d(buff);
    mm = conv2d(buff + 3);
    ss = conv2d(buff + 6);
}

uint8_t DateTime::dayOfTheWeek() const {    
    uint16_t day = date2days(yOff, m, d);
    return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

uint32_t DateTime::unixtime(void) const {
  uint32_t t;
  uint16_t days = date2days(yOff, m, d);
  t = time2long(days, hh, mm, ss);
  t += SECONDS_FROM_1970_TO_2000;  // seconds from 1970 to 2000

  return t;
}

long DateTime::secondstime(void) const {
  long t;
  uint16_t days = date2days(yOff, m, d);
  t = time2long(days, hh, mm, ss);
  return t;
}

DateTime DateTime::operator+(const TimeSpan& span) {
  return DateTime(unixtime()+span.totalseconds());
}

DateTime DateTime::operator-(const TimeSpan& span) {
  return DateTime(unixtime()-span.totalseconds());
}

TimeSpan DateTime::operator-(const DateTime& right) {
  return TimeSpan(unixtime()-right.unixtime());
}

////////////////////////////////////////////////////////////////////////////////
// TimeSpan implementation

TimeSpan::TimeSpan (int32_t seconds):
  _seconds(seconds)
{}

TimeSpan::TimeSpan (int16_t days, int8_t hours, int8_t minutes, int8_t seconds):
  _seconds((int32_t)days*86400L + (int32_t)hours*3600 + (int32_t)minutes*60 + seconds)
{}

TimeSpan::TimeSpan (const TimeSpan& copy):
  _seconds(copy._seconds)
{}

TimeSpan TimeSpan::operator+(const TimeSpan& right) {
  return TimeSpan(_seconds+right._seconds);
}

TimeSpan TimeSpan::operator-(const TimeSpan& right) {
  return TimeSpan(_seconds-right._seconds);
}

///////////////////////////////////////////////////////////////////////////////
// BCD conversion tools

static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }

////////////////////////////////////////////////////////////////////////////////
// RTC_PCF8563 implementation

boolean clock::begin(void) {
  Wire.begin();
  return true;
}

boolean clock::initialized(void) {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE((byte)PCF8523_CONTROL_3);
  Wire.endTransmission();

  Wire.requestFrom(PCF8523_ADDRESS, 1);
  uint8_t ss = Wire._I2C_READ();
  return ((ss & 0xE0) != 0xE0);
}

void clock::adjust(const DateTime& dt) {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE((byte)3); // start at location 3
  Wire._I2C_WRITE(bin2bcd(dt.second()));
  Wire._I2C_WRITE(bin2bcd(dt.minute()));
  Wire._I2C_WRITE(bin2bcd(dt.hour()));
  Wire._I2C_WRITE(bin2bcd(dt.day()));
  Wire._I2C_WRITE(bin2bcd(0)); // skip weekdays
  Wire._I2C_WRITE(bin2bcd(dt.month()));
  Wire._I2C_WRITE(bin2bcd(dt.year() - 2000));
  Wire.endTransmission();

  // set to battery switchover mode
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE((byte)PCF8523_CONTROL_3);
  Wire._I2C_WRITE((byte)0x00);
  Wire.endTransmission();
}

DateTime clock::now() {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE((byte)3); 
  Wire.endTransmission();

  Wire.requestFrom(PCF8523_ADDRESS, 7);
  uint8_t ss = bcd2bin(Wire._I2C_READ() & 0x7F);
  uint8_t mm = bcd2bin(Wire._I2C_READ());
  uint8_t hh = bcd2bin(Wire._I2C_READ());
  uint8_t d = bcd2bin(Wire._I2C_READ());
  Wire._I2C_READ();  // skip 'weekdays'
  uint8_t m = bcd2bin(Wire._I2C_READ());
  uint16_t y = bcd2bin(Wire._I2C_READ()) + 2000;
  
  return DateTime (y, m, d, hh, mm, ss);
}

Pcf8523SqwPinMode clock::readSqwPinMode() {
  int mode;

  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE(PCF8523_CLKOUTCONTROL);
  Wire.endTransmission();
  
  Wire.requestFrom((uint8_t)PCF8523_ADDRESS, (uint8_t)1);
  mode = Wire._I2C_READ();

  mode >>= 3;
  mode &= 0x7;
  return static_cast<Pcf8523SqwPinMode>(mode);
}

void clock::writeSqwPinMode(Pcf8523SqwPinMode mode) {
  Wire.beginTransmission(PCF8523_ADDRESS);
  Wire._I2C_WRITE(PCF8523_CLKOUTCONTROL);
  Wire._I2C_WRITE(mode << 3);
  Wire.endTransmission();
}