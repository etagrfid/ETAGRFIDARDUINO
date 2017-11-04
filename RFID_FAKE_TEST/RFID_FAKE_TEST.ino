/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "Timer.h"
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
typedef struct{
  bool parity:1;
  byte data_nibb:4;
  byte xxxxx:3;
  } EM4100Line;

typedef struct{
  EM4100Line lines[10];
  //EM4100Parity parity;
  bool stop_bit:1;
  byte colparity:4;
  byte xxxx:3;
} EM4100Data;
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

    if(bitPosition == 54)
    {
      gPacketBufWrite = 0;
      headerFound     = 0;
      header_count    = 0;
    }
  }
theEnd:     
  //assingments to happen at the end
  gReadBitCount++;

  lastBit = bval;
}
uint8_t iFakeData[] = {
                0b00011,
                0b10111, //version number / customer id + even parity column
                0b10111,
                0b01111,
                0b00011, //Data bits + even parity column
                0b10100,
                0b10111,
                0b10111,
                0b10010,
                0b10111,
                0b11110};// Column Parity bits and stop bit (0)

bool gFakeData[] = {// 1,1,1,1,1,1,1,1,1,
                0,0,0,1,1,
                1,0,1,1,1, //version number / customer id + even paraty column
                1,0,1,1,1,
                0,1,1,1,1,
                0,0,0,1,1, //Data bits + even paraty column
                1,0,1,0,0,
                1,0,1,1,1,
                1,0,1,1,1,
                1,0,0,1,0,
                1,0,1,1,1,
                1,1,1,1,0};// Column Parity bits and stop bit (0)
                
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
    pinMode(pLED,OUTPUT);
    digitalWrite(pLED,0);
	pinMode(outputpin, OUTPUT);
	digitalWrite(outputpin, HIGH);
	pinMode(demodOut,INPUT); 
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
  
	delay(10);
	/*for (int i = 0; i < num_bits; i++) {
	data[i] = (0 == i % 2);
	}*/
	//for M0
	//attachInterrupt(demodOut, INT_manchesterDecode, CHANGE);
	//MEGA
	attachInterrupt(digitalPinToInterrupt(demodOut), INT_manchesterDecode, CHANGE);
	delay(10);
	startTimer(4);
}
int has_even_parity(uint16_t x,int datasize)
{
  volatile unsigned int count = 0, i, b = 1;

  for(volatile int i = 0; i < datasize; i++){
    if( x & (b << i) ){count++;}
  }

  if( (count % 2) ){return 0;}

  return 1;
}

int CheckManchesterParity(EM4100Data *xd)
{
  int row_err_count=0;
  int col_err_count=0;
  int err_count = 0;
  for(int i=0;i<10;i++)
  {
    bool this_row = !has_even_parity(xd->lines[i].data_nibb,4);
    if(this_row != xd->lines[i].parity)
      row_err_count++;
  }
  for(int i=0;i<4;i++)
  {
    volatile uint16_t coldata = 0x00;
    uint16_t imask = 0x01 << i;
    for(int ii=0;ii<10;ii++)
	{
		uint16_t thisbit = xd->lines[ii].data_nibb;
		thisbit &= imask;
		thisbit >>= i;
		coldata |= thisbit  << ii;
	}
	
    bool this_colp = !has_even_parity(coldata,10);
	volatile uint8_t readparity = xd->colparity;
    uint8_t read_col_par_bit = (readparity & imask) >> i;
    if(this_colp != read_col_par_bit)
      col_err_count++;
  }
  err_count = col_err_count+row_err_count;
}
void loop() 
{
  static int foo = 2;
    

  //if(foo <= 0)
    return;
  foo--;
  
  delay(750);
  startTimer(1);
  //tcConfigure(250);
  //transmit(gFakeData,sizeof(gFakeData));
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
  /*for(uint32_t i=0;i<gReadBitCount;i++)
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
  }*/
  //serial.print("Bit errors: ");
  //serial.println(errors);
  //errors=0;
 /*for(uint32_t i=0;i<sizeof(gPacketBufWithParity);i++)
  {
      serial.print(0b00011111 & (uint32_t)gPacketBufWithParity[i],BIN);
      serial.print(", ");
      serial.println((uint32_t)iFakeData[i],BIN);
  }*/
  serial.println("READ");
   EM4100Data *xd = (EM4100Data*)gPacketBufWithParity;
   //look at parity rows
   /*for(int i=0;i<10;i++)
   {
    serial.print(xd->lines[i].data_nibb,BIN);
    serial.print(", ");
    serial.print(xd->lines[i].parity);
    serial.print(", ");
    serial.println(!has_even_parity(xd->lines[i].data_nibb,4));
   }*/
   int pcheck = CheckManchesterParity(xd);
   serial.print("Parity: ");
   serial.println(pcheck);

	serial.print("Data: ");
   for(int i=0;i<10;i+=2)
   {
		uint8_t data0 = (xd->lines[i].data_nibb << 4) | xd->lines[i+1].data_nibb;
		serial.print(data0,HEX);
		if(i<8)
			serial.print(",");
   }
   serial.println();
}