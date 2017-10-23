/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
/*
#include "ArduinoLowPower.h"
ArduinoLowPowerClass LowPower;



//Beginning of Auto generated function prototypes by Atmel Studio
void dummy();
//End of Auto generated function prototypes by Atmel Studio



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Uncomment this function if you wish to attach function dummy when RTC wakes up the chip
  // LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, dummy, CHANGE);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  // Triggers a 2000 ms sleep (the device will be woken up only by the registered wakeup sources and by internal RTC)
  // The power consumption of the chip will drop consistently
  LowPower.sleep(2000);
}

void dummy() {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
}*/
#define serial Serial
#define num_bits 64
#define outputpin 2
#define pLED 13

//2, 3, 18, 19, 20, 21 for MEGA
//2, 3 for UNO
//ALL for M0
#define demodOut 8
//#define shd 8
#define sendDelay 250
#define pskDelay 290
uint8_t xxxxxx;
//Book keeping vars
volatile uint32_t gReadBitCount = 0;
volatile uint32_t gIntCount = 0;
#define gbRingBuffSize 1024
volatile uint8_t    gbRingBuff[gbRingBuffSize];
volatile uint32_t   gRingBufWrite = 0;
volatile uint32_t   gRingBufRead = 0;

volatile uint8_t    gPacketBufWithParity[11];
volatile uint32_t   gPacketBufWrite=0;
bool ReadFromRB(void)
{
  bool retval =  gbRingBuff[gRingBufRead++];
  if(gRingBufRead > gbRingBuffSize)
    gRingBufRead = 0;
  return retval;
}
void INT_manchesterDecode() 
{
  static bool headerFound = false;

  unsigned long curr_time = micros();
  static unsigned long prev_time = 0;
  
  unsigned long time_diff = curr_time - prev_time;
  prev_time = curr_time;
  //serial.println(time_diff);
  
  
  bool PinState = digitalRead(demodOut);
  unsigned short thisRead = 0;
  if(PinState)
    thisRead = 1;
   
  static uint8_t lastRead = thisRead;
  static uint8_t lastBit = 0;

  uint8_t bval = 0;

  if(time_diff > pskDelay*2)
  {
    lastRead = thisRead;
    lastBit = 0;
  }
  if(time_diff > pskDelay)
  {
    if(gIntCount > 1)
    {
      if(lastRead == 1 && thisRead == 0)
        bval = 0;
      else if(lastRead == 0 && thisRead == 1)
        bval = 1;
    }
    else
    {
      if(thisRead == 0)
        bval = 0;
      else
        bval = 1;
      goto waitfornextInt;
    }
  }
  else
  {
    if(gIntCount <= 1)
    {      
      if(thisRead == 1)
      {
        bval = 1;
      }
      else if(thisRead == 0)
      {
        bval = 0;
      }
    }
    else
    {
      if(lastRead == 0 && thisRead == 1 && lastBit == 0)
      {
        goto waitfornextInt;
      }
      else if(lastRead == 1 && thisRead == 0 && lastBit == 1)
      {
        goto waitfornextInt;
      }
      else if(lastRead == 0 && thisRead == 1)
      {
        bval = 1;
      }
      else if(lastRead == 1 && thisRead == 0)
      {
        bval = 0;
      }
    }
  }
  goto keep_exe_int;
waitfornextInt:  
  lastRead = thisRead;
  //keep track of interrupts
  gIntCount++;
  return;
keep_exe_int:
  //serial.print("r ");
  //serial.println(bval);
  //keep track of interrupts

  gIntCount++;
  lastRead = thisRead;
  gbRingBuff[gRingBufWrite++] = bval;
  if(gRingBufWrite > gbRingBuffSize)
    gRingBufWrite=0;
    
  static uint8_t header_count = 0;
  if(bval == 1 && gReadBitCount == 0)
  {
    header_count++;
  }
  else if(bval == 1 && lastBit == 1)
  {
    header_count++;
  }
  else
    header_count = 0;
    
  if(header_count > 8)
  {
    digitalWrite(pLED,1);
    headerFound = 1;
    header_count = 0;
    goto theEnd;

  }

  if(headerFound)
  { 
    //
    int bitPosition = gReadBitCount-9;//starts at 0
    //if(bitPosition > 4)
    //  goto theEnd;
    //uint8_t needsShifted = bitPositon % 5;
    //uint8_t newBitShifted = bval << needsShifted;
  gPacketBufWithParity[gPacketBufWrite] <<= 1;
    gPacketBufWithParity[gPacketBufWrite] |= bval;

    //increment our writer
    if((bitPosition+1) % 5 == 0)//parity at 5,10....50
    {
      gPacketBufWrite++;
      gPacketBufWithParity[gPacketBufWrite] = 0x00;
    }
    //if(gPacketBufWrite>=sizeof(gPacketBufWithParity))
    //  gPacketBufWrite=0;

    /*if(bitPositon == 54)
    {
      gPacketBufWrite = 0;
      headerFound     = 0;
      header_count    = 0;
    }*/
  }
theEnd:     
  //assingments to happen at the end
  gReadBitCount++;

  lastBit = bval;
}
uint8_t iFakeData[] = {
                0b00011,
                0b10110, //version number / customer id + even parity column
                0b10110,
                0b01110,
                0b00011, //Data bits + even parity column
                0b10100,
                0b10111,
                0b10111,
                0b10010,
                0b10111,
                0b00101};// Column Parity bits and stop bit (0)

bool gFakeData[] = {// 1,1,1,1,1,1,1,1,1,
                0,0,0,1,1,
                1,0,1,1,0, //version number / customer id + even paraty column
                1,0,1,1,0,
                0,1,1,1,0,
                0,0,0,1,1, //Data bits + even paraty column
                1,0,1,0,0,
                1,0,1,1,1,
                1,0,1,1,1,
                1,0,0,1,0,
                1,0,1,1,1,
                0,0,1,0,1};// Column Parity bits and stop bit (0)
                
void transmit(bool *idata,int totalBits) 
{
  //int slamcount=0;
  int mics = sendDelay;
  int d1 = mics;
  int d2 = mics;
  int s1,s2;
  //insert 9-1's for the header
  for (int i = 0; i < 9; i++) 
  {
    digitalWrite(outputpin, 0);
    delayMicroseconds(d1);
    digitalWrite(outputpin, 1);
    delayMicroseconds(d2);
  }
  for (int i = 0; i < totalBits; i++) 
  {
    if (idata[i])
    {
      s1 = 0;
      s2 = 1;
    }
    else if (~idata[i])
    {
      s1 = 1;
      s2 = 0;
    }
    digitalWrite(outputpin, s1);
    delayMicroseconds(d1);
    digitalWrite(outputpin, s2);
    delayMicroseconds(d2);
    //slamcount+=2;
  }
  //serial.print("Total flips: ");
  //serial.println(slamcount);
  delay(100);
  digitalWrite(outputpin, HIGH);
}
void setup() 
{
  // put your setup code here, to run once:
  serial.begin(230400);
  delay(100);

  //while (!serial);
  serial.println("running");
  serial.print("UINT SIZE:");
  serial.println(sizeof(unsigned int));
  serial.print("ULONG SIZE:");
  serial.println(sizeof(unsigned long));
  serial.print("Data Size: ");
  serial.println(sizeof(gFakeData));
  pinMode(outputpin, OUTPUT);
  digitalWrite(outputpin, HIGH);
  delay(10);
  /*for (int i = 0; i < num_bits; i++) {
    data[i] = (0 == i % 2);
  }*/
  pinMode(demodOut,INPUT);
  //for M0
  //attachInterrupt(demodOut, INT_manchesterDecode, CHANGE);
  //MEGA
  attachInterrupt(digitalPinToInterrupt(demodOut), INT_manchesterDecode, CHANGE);
  delay(10);

  pinMode(pLED,OUTPUT);
  digitalWrite(pLED,0);
}
char sBuf[256];
void loop() 
{
  static int foo = 1;
  
  if(foo <= 0)
    return;
  foo--;
  delay(750);
  transmit(gFakeData,sizeof(gFakeData));
  delay(750);
  digitalWrite(pLED,0);

  serial.print("I: ");
  serial.print(gIntCount);
  serial.print(" W: ");
  serial.print(gRingBufWrite);
  serial.print(" R: ");
  serial.print(gRingBufRead);
  serial.println();
  gIntCount = 0;
  uint32_t errors = 0;
  for(uint32_t i=0;i<gReadBitCount;i++)
  {
      unsigned short newbyte = ReadFromRB();
      int fakeme = 1;
      if(i >= 9)
        fakeme = (int)gFakeData[i-9];
      //int fakeme = (int)gFakeData[i];
      serial.print(i);
      serial.print(", ");
      serial.print(newbyte);
      serial.print(", ");
      serial.print(fakeme);
      serial.println();
      if(newbyte != fakeme)
        errors++;
  }
  serial.print("Bit errors: ");
  serial.println(errors);
  errors=0;
  for(uint32_t i=0;i<sizeof(gPacketBufWithParity);i++)
  {
      serial.print((uint32_t)gPacketBufWithParity[i],BIN);
      serial.print(", ");
      serial.println((uint32_t)iFakeData[i],BIN);
  }
  
  gReadBitCount=0; 
}
