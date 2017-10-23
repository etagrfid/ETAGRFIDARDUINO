#define serial Serial
#define num_bits 64
#define outputpin 2
#define pLED 13

//2, 3, 18, 19, 20, 21 for MEGA
//2, 3 for UNO
//ALL for M0
#define demodOut 8
//#define shd 8

//Book keeping vars
volatile unsigned long gReadBitCount = 0;
volatile unsigned long gIntCount = 0;
#define gbRingBuffSize 1024
volatile unsigned short   gbRingBuff[gbRingBuffSize];
volatile int              gRingBufWrite = 0;
volatile int              gRingBufRead = 0;

bool ReadFromRB(void)
{
  bool retval =  gbRingBuff[gRingBufRead++];
  if(gRingBufRead > gbRingBuffSize)
    gRingBufRead = 0;
  return retval;
}
void INT_manchesterDecode() 
{
  unsigned long curr_time = micros();
  static unsigned long prev_time = 0;
  //keep track of interrupts
  gIntCount++;
  unsigned long time_diff = curr_time - prev_time;
  prev_time = curr_time;
  
  //save the time difference
  
  
  bool PinState = digitalRead(demodOut);
  unsigned short thisRead = 0;
  if(PinState)
    thisRead = 1;
   
  static unsigned short lastRead = 0;
  unsigned short bval = 0;
  static unsigned short lastBit = 0;

  if(time_diff > 280)
  {
    if(gIntCount >= 0)
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
    }
  }
  else
  {
    if(gIntCount <= 1)
    {
      if(lastRead == 0 && thisRead == 1)
      {
        bval = 1;
      }
      else if(lastRead == 1 && thisRead == 0)
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
  lastBit = bval;
  goto keep_exe_int;
waitfornextInt:  
  lastRead = thisRead;
  return;
keep_exe_int:
  lastRead = thisRead;
  gReadBitCount++;
  gbRingBuff[gRingBufWrite++] = bval;
  if(gRingBufWrite > sizeof(gbRingBuff))
    gRingBufWrite=0;

}


bool gFakeData[] = { 1,1,1,1,1,1,1,1,1,
                0,0,0,1,1,0,1,1,
                1,0,1,1,0, //version number / customer id + even paraty column
                1,0,1,1,0,
                0,1,1,1,0,
                0,0,0,1,1, //Data bits + even paraty column
                1,0,1,0,0,
                1,0,1,1,1,
                1,0,1,1,1,
                1,0,0,1,0,
                1,0,1,1,1,
                0,0,1,0,1,
                1,0,1,1,1,
                0,0,0,0,0};// Column Parity bits and stop bit (0)

void sendBit(int mics, bool idata) 
{
  
}

void transmit(bool *idata,int totalBits) 
{
  int slamcount=0;
  for (int i = 0; i < totalBits; i++) 
  {
    int mics = 250;
    int d1 = mics;
    int d2 = mics;
    int s1,s2;
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
    delayMicroseconds(mics);
    digitalWrite(outputpin, s2);
    delayMicroseconds(mics);
    slamcount+=2;
  }
  serial.print("Total flips: ");
  serial.println(slamcount);
}
void setup() 
{
  // put your setup code here, to run once:
  serial.begin(115200);
  //while (!serial);
  serial.println("running");
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
  static int foo = 2;
  
  if(foo <= 0)
    return;
  foo--;
  transmit(gFakeData,sizeof(gFakeData));
  delay(500);
  
  serial.print("I: ");
  serial.print(gIntCount);
  serial.print(" W: ");
  serial.print(gRingBufWrite);
  serial.print(" R: ");
  serial.print(gRingBufRead);
  serial.println();
  gIntCount = 0;
  for(int i=0;i<gReadBitCount;i++)
  {
      unsigned short newbyte = ReadFromRB();
      serial.print(i);
      serial.print(", ");
      serial.print(newbyte);
      serial.print(", ");
      serial.print((int)gFakeData[i]);
      serial.println();
  }
  gReadBitCount=0; 
}
