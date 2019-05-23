## ETAGRFIDARDUINO
### Arduino software to log RFID tag scans
#### How to install:
- If you downloaded a zipped version of this repositoru, you will probably need to rename the folder to match the name of the .ino file (minus the ".ino"). So the folder should be renamed to "ETAG_4095"
- The library files (e.g. "SparkFun_RV1805.cpp" and "SparkFun_RV1805.h") need to be in the same folder as the .ino file or they can be manually moved into your 'libraries' folder in arduino (Ususally somethign like ...Documents/Arduino/Libraries.
- in future versions the library will be availible through the library manager in Arduino
#### Examples:
- Combined:
    - This Example uses all features of the libary to log the time, date, ID, and data for every tag scan the reader picks up. This also uses the settings feature which allows the user to specify various settings via a settings file on an SD card that loads on boot.
- RTC:
    - This example allows the user to make use of the on board real time clock
- RFID: 
    - This example allows the user to read RFID tags.
