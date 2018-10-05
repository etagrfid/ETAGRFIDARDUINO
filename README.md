THIS IS THE ETAG LOW POWER TEST PROGRAM
LED will blink after 10 seconds for ~10 seconds and then sleep
External button will wakeup also
USB will be disabled and re-enabled (serial data output will occur if connected)

## ETAGRFIDARDUINO
### Arduino software to log RFID tag scans
#### How to install:
- Currently this repository requires you to manually move the library into your 'libraries' folder in arduino.
- in future versions the library will be availible through the library manager in Arduino
#### Examples:
- Combined:
    - This Example uses all features of the libary to log the time, date, ID, and data for every tag scan the reader picks up. This also uses the settings feature which allows the user to specify various settings via a settings file on an SD card that loads on boot.
- RTC:
    - This example allows the user to make use of the on board real time clock
- RFID: 
    - This example allows the user to read RFID tags.
