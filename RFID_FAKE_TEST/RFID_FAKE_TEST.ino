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
#define pskDelay 280
uint8_t xxxxxx;
//Book keeping vars
volatile uint32_t gReadBitCount = 0;
volatile uint32_t gIntCount = 0;
#define gbRingBuffSize 128
volatile uint8_t    gbRingBuff[gbRingBuffSize];
volatile uint32_t   gRingBufWrite = 0;
volatile uint32_t   gRingBufRead = 0;

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
  
  unsigned long time_diff = curr_time - prev_time;
  prev_time = curr_time;
  //serial.println(time_diff);

  //save the time difference
  
  
  bool PinState = digitalRead(demodOut);
  unsigned short thisRead = 0;
  if(PinState)
    thisRead = 1;
   
  static unsigned short lastRead = thisRead;
  unsigned short bval = 0;
  static unsigned short lastBit = 0;
  

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
  lastBit = bval;
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
  gReadBitCount++;
  gbRingBuff[gRingBufWrite++] = bval;
  if(gRingBufWrite > gbRingBuffSize)
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
void transmit(bool *idata,int totalBits) 
{
  int slamcount=0;
  for (int i = 0; i < totalBits; i++) 
  {
    int mics = sendDelay;
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
    delayMicroseconds(d1);
    digitalWrite(outputpin, s2);
    delayMicroseconds(d2);
    slamcount+=2;
  }
  serial.print("Total flips: ");
  serial.println(slamcount);
}
void setup() 
{
  // put your setup code here, to run once:
  serial.begin(230400);
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
  static int foo = 2;
  
  if(foo <= 0)
    return;
  foo--;
  delay(750);
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
  uint32_t errors = 0;
  for(unsigned long i=0;i<gReadBitCount;i++)
  {
      unsigned short newbyte = ReadFromRB();
      serial.print(i);
      serial.print(", ");
      serial.print(newbyte);
      serial.print(", ");
      serial.print((int)gFakeData[i]);
      serial.println();
      if(newbyte != gFakeData[i])
        errors++;
  }
  serial.print("Errors: ");
  serial.println(errors);
  errors=0;
  gReadBitCount=0; 
}
