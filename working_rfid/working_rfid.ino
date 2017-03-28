#include <RFIDuino.h> //include the RFIDuino Library

#define SERIAL_PORT Serial      //Serial port definition for Geekduino, Arduino Uno, and most Arduino Boards
RFIDuino myRFIDuino(1.2);   //pins are set for the board in the library

byte tagData[5]; //Holds the ID numbers from the tag
unsigned long tagID;

void setup()
{
  //begin serial communicatons at 9600 baud and print a startup message
  SERIAL_PORT.begin(9600);
  SERIAL_PORT.println("Welcome to the RFIDuino Serial Example. Please swipe your RFID Tag.");

  //The RFIDUINO hardware pins and user outputs(Buzzer / LEDS) are all initialized via pinMode() in the library initialization, so you don not need to to that manually
}

void loop()
{
  //scan for a tag - if a tag is sucesfully scanned, return a 'true' and proceed
  if (myRFIDuino.scanForTag(tagData) == true){
    SERIAL_PORT.print("RFID Tag Data: "); //print a header to the Serial port.
    //loop through the byte array
    for (int n = 0; n < 32; n++){
      SERIAL_PORT.print(tagData[n], HEX);
    }
    SERIAL_PORT.print("\n\rRFID Tag ID: ");
    tagID = ((long)tagData[1] << 24) + ((long)tagData[2] << 16) + ((long)tagData[3] << 8) + tagData[4];
    SERIAL_PORT.print(tagID);
    SERIAL_PORT.print("\n\r");//return character for next line
    delay(100);
  }
}// end loop()
