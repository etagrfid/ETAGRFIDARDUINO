/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "Timer.h"
#include "ManchesterUtil.h"



#define serial Serial
#define num_bits 64
#define outputpin 2
#define pLED 13
#define MANCHESTER_PACKET_SIZE 55
//2, 3, 18, 19, 20, 21 for MEGA
//2, 3 for UNO
//ALL for M0
#define demodOut 8
#define triggerOutPin 12
//#define shd 8


ManchesterIntInfo gIntMachine;
#define zShortLow 150
#define zShortHigh 310

#define zLongLow 400
#define zLongHigh 600
int gFoundPackets = 0;
volatile uint8_t    gClientPacketBufWithParity[11];
volatile uint8_t	gPacketRead = 0;

#define nBitRingBufLength 1024
uint8_t tDiffPinBuf[nBitRingBufLength];
uint16_t dWriteIndex = 0;
uint16_t dReadIndex = 0;
uint16_t dDataCount = 0;

int HandleIntManchester(int8_t fVal, int8_t fTimeClass);
//int HandleManchesterInt(int fVal, uint32_t milli_time);

void INT_manchesterDecode(void)
{
	volatile uint32_t timeNow = micros();
	volatile static uint32_t lastTime = 0;
	uint32_t fDiff = timeNow - lastTime;
	lastTime = timeNow;
	int8_t fTimeClass = ManchesterIntInfo::tUnknown;
	int8_t fVal = !digitalRead(demodOut);
	
	if (fDiff >= zShortLow && fDiff <= zShortHigh)
		fTimeClass = ManchesterIntInfo::tShort;
	else if ((fDiff >= zLongLow && fDiff <= zLongHigh))
		fTimeClass = ManchesterIntInfo::tLong;
	else if (fDiff < 10)
		return;

	tDiffPinBuf[dWriteIndex] = 0;
	tDiffPinBuf[dWriteIndex] = 0xF0 & (fTimeClass << 4);
	tDiffPinBuf[dWriteIndex++] |= 0x0F & fVal;
	if (dWriteIndex >= nBitRingBufLength)
		dWriteIndex = 0;
	dDataCount++;
}

int HandleIntManchester(int8_t fVal, int8_t fTimeClass)
{
	if (fTimeClass != ManchesterIntInfo::tLong && fTimeClass != ManchesterIntInfo::tShort)
		return 0;

	//printf("%i T - %d P %d SS %d ", gIntMachine.intCount, fTimeClass, fVal, gIntMachine.syncState);

	if (gIntMachine.syncState == 0 &&
	fVal == 0 && gIntMachine.lastValue == 1 && gIntMachine.secondLastValue == 0 &&
	fTimeClass == ManchesterIntInfo::tShort && gIntMachine.lastTimeClass == ManchesterIntInfo::tLong)
	{
		//printf("Sync at %d", gIntMachine.intCount);
		gIntMachine.syncState = 1;
		gIntMachine.headerCount = 1;
		gIntMachine.headerFound = 0;
		//dataCap = [];
		//headerLocation = i;
		//printf("\n");
	}
	if (gIntMachine.syncState == 1 && fVal == 1 && gIntMachine.secondLastValue == 1 && fTimeClass == ManchesterIntInfo::tShort && gIntMachine.lastTimeClass == ManchesterIntInfo::tShort)
	{
		gIntMachine.headerCount++;
		//printf("Up Header at %d, %d", gIntMachine.intCount, gIntMachine.headerCount);
		if (gIntMachine.headerCount == 9 && gIntMachine.headerFound == 0)
		{
			//printf(" Header at %d\n", gIntMachine.intCount);
			gIntMachine.headerFound = 1;
			gIntMachine.syncState = 2;
			//gIntMachine.headerLocation = i - 15;
			//return gIntMachine.UpdateMachine(pinRead, timeNow, timeClass);
		}
		//else
		//printf("\n");
	}
	else if(gIntMachine.syncState == 1 && fTimeClass == ManchesterIntInfo::tLong)
	{
		//printf("Reset at %d", gIntMachine.intCount);
		gIntMachine.ResetMachine();
		return gIntMachine.UpdateMachineUsingClass(fVal, fTimeClass);
	}
	if (gIntMachine.syncState <= 1)
	{
		//printf("\n");
		return gIntMachine.UpdateMachineUsingClass(fVal, fTimeClass);
	}

	int newBit = 0;
	if (gIntMachine.syncState == 2 && gIntMachine.headerFound == 1 && fTimeClass == ManchesterIntInfo::tShort)
	{
		gIntMachine.syncState = 3;
		//printf("%d Skip Short %d\n", gIntMachine.intCount, fVal);
	}
	else if (gIntMachine.syncState == 3 && gIntMachine.headerFound == 1 && fTimeClass == ManchesterIntInfo::tShort)
	{
		gIntMachine.syncState = 2;
		//printf("%d Keep Short %d\n", gIntMachine.intCount, fVal);
		gIntMachine.StoreNewBit(fVal);
		//dataCap = [dataCap; fVal];
		newBit = 1;
	}
	else if ((gIntMachine.syncState == 2 || gIntMachine.syncState == 3) && gIntMachine.headerFound == 1 && fTimeClass == ManchesterIntInfo::tLong)
	{
		gIntMachine.syncState = 2;
		//printf("%d Keep Long %d\n", gIntMachine.intCount, fVal);
		//dataCap = [dataCap; fVal];
		gIntMachine.StoreNewBit(fVal);
		newBit = 1;
	}
	else
	//printf("Dead at %d\n", gIntMachine.intCount);
	

	if ((gIntMachine.dataBinCount-1 % 5) == 0 && gIntMachine.dataBinCount -1 > 1)
	{
		uint8_t checkD = gIntMachine.dataBuf[gIntMachine.dataBufWrite-1];
		uint8_t pCalc = 0;
		for (int i = 0; i < 4; i++)
		pCalc = 0x01 & (checkD >> i);
		if (pCalc != (0x01 & checkD))
		{
			//printf("Error at parity %d\n", gIntMachine.intCount);
			gIntMachine.ResetMachine();
		}
		//(checkD >> 1) %2 != (0x01 & checkD)

	}

	if (gIntMachine.dataBinCount >= 55)
	{
		if(gPacketRead == 0)
		{
			memset((uint8_t*)gClientPacketBufWithParity,0x00,sizeof(gClientPacketBufWithParity));
			memcpy((uint8_t*)gClientPacketBufWithParity,(uint8_t*)gIntMachine.dataBuf,sizeof(gIntMachine.dataBuf));
			gPacketRead = 1;
		}
		//EM4100Data *foo = (EM4100Data*)gIntMachine.dataBuf;
		//printf("End of packet at %d\n", gIntMachine.intCount);
		gIntMachine.ResetMachine();
		gIntMachine.UpdateMachineUsingClass(fVal, fTimeClass);
		return gFoundPackets++;
	}

	return gIntMachine.UpdateMachineUsingClass(fVal, fTimeClass);
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

bool gFakeData[] = {
					/*0,0,0,0,0,
					0,0,0,0,0, 
					0,0,0,0,0,
					0,0,0,0,0,
					0,0,0,0,0,
					0,0,0,0,0,
					0,0,0,0,0, 
					0,0,0,0,0,
					0,0,0,0,0,
					0,0,0,1,1,
					0,0,0,1,0};*/
					1,1,1,1,0,
					0,1,1,0,0, 
					0,0,0,0,0,
					0,0,0,0,0,
					0,1,0,1,0,
					0,0,1,1,0,
					1,1,1,0,1,
					1,0,0,0,1,
					1,0,0,1,0,
					0,0,0,1,1,
					0,0,0,1,0};
	
	/*{// 1,1,1,1,1,1,1,1,1,
                0,0,0,1,1,
                1,0,1,1,1, //version number / customer id + even parity column
                1,0,1,1,1,
                0,1,1,1,1,
                0,0,0,1,1, //Data bits + even parity column
                1,0,1,0,0,
                1,0,1,1,1,
                1,0,1,1,1,
                1,0,0,1,0,
                1,0,1,1,1,
                1,1,1,1,0};*/// Column Parity bits and stop bit (0)
bool noiseBits[] = {1,1,1,1,1,0,0,1,0,1,0,1,0,1,1,1,1,1,1}; 
#define sendDelay		250
   
void transmit(bool *idata,int totalBits,bool *noiseBits,int numberNoiseBits) 
{
  //int slamcount=0;
  int mics = sendDelay;
  int d1 = mics;
  int d2 = mics;
  int s1,s2;
  digitalWrite(outputpin, 0);
  delayMicroseconds(250);

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
    else if (!idata[i])
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
	//delay(10);
	//digitalWrite(outputpin, HIGH);
	

	/*for (int i = 0; i < numberNoiseBits; i++)
	{
		if (noiseBits[i])
		{
			s1 = 0;
			s2 = 1;
		}
		else if (~noiseBits[i])
		{
			s1 = 1;
			s2 = 0;
		}
		digitalWrite(outputpin, s1);
		delayMicroseconds(d1*3);
		digitalWrite(outputpin, s2);
		delayMicroseconds(d2*3);
		//slamcount+=2;
	}*/
  //serial.print("Total flips: ");
  //serial.println(slamcount);
  delay(2);
  digitalWrite(outputpin, HIGH);
}
volatile int timerFakeState = 0;
volatile int timerBitPointer = 0;
volatile int timerHeaderCount = 0;
void TC3_Handler() {
	TcCount16* TC = (TcCount16*) TC3;
	// If this interrupt is due to the compare register matching the timer count
	// we toggle the LED.
	static bool isLEDOn=false;
	if (TC->INTFLAG.bit.MC0 == 1) {
		TC->INTFLAG.bit.MC0 = 1; //clear the overflow flag
		
		
		static uint64_t lastInt =	millis();
		uint64_t nowInt  =	millis();
		uint64_t elapsedT = nowInt - lastInt;
		lastInt = nowInt;
		if(elapsedT > sendDelay)//wait more than the shift delay to reset
		{
			timerFakeState=timerHeaderCount=timerBitPointer=0;
			digitalWrite(outputpin, HIGH);
		}
		
		if(timerHeaderCount < 9)
		{
			digitalWrite(pLED, HIGH);
			if(timerFakeState == 0)
			{
				digitalWrite(outputpin, LOW);
				timerFakeState = 1;
			}
			else if(timerFakeState == 1)
			{	
				digitalWrite(outputpin, HIGH);
				timerFakeState = 0;
				timerHeaderCount++;
			}
		}
		else if(timerBitPointer < MANCHESTER_PACKET_SIZE)
		{
			uint8_t fakeBit = gFakeData[timerBitPointer];
			if(timerFakeState == 0)
			{
				digitalWrite(outputpin, !fakeBit);
				timerFakeState = 1;
			}
			else if(timerFakeState == 1)
			{	
				digitalWrite(outputpin, fakeBit);
				timerFakeState = 0;
				timerBitPointer++;
			}
		}
		else 
		{
			static int fixed_delay = 0;
			
			if(fixed_delay < 4000)
			{
				fixed_delay++;	
			}
			else
			{
				fixed_delay = 0;
				timerFakeState=timerHeaderCount=timerBitPointer=0;
				digitalWrite(outputpin, HIGH);
			}
			if(fixed_delay == 1 || fixed_delay >= 3800)
			{
				digitalWrite(outputpin, HIGH);
				digitalWrite(pLED, LOW);
			}
			else if(fixed_delay > 1 && fixed_delay < 3999)
			{
				static int rand_count = 0;
				if(rand_count <= 0)
				{
					rand_count = random(1,20);
					digitalWrite(pLED, isLEDOn);
					digitalWrite(outputpin, random(0,1));

					isLEDOn = !isLEDOn;			
				}
				else
					rand_count--;
			}
			//TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
			//timerFakeState=timerHeaderCount=timerBitPointer=0;
		}
	}
}
void setup() 
{
    //Shutdown pin
	pinMode(7,OUTPUT);
    digitalWrite(7,0);
	//MOD PIN
	pinMode(6,OUTPUT);
	digitalWrite(6,0);
	pinMode(triggerOutPin,OUTPUT);
	digitalWrite(triggerOutPin,LOW);
    pinMode(pLED,OUTPUT);
    digitalWrite(pLED,0);
	pinMode(outputpin, OUTPUT);
	digitalWrite(outputpin, HIGH);
	pinMode(demodOut,INPUT); 
	serial.begin(115200);//500000);//230400);
	delay(100);
	randomSeed(analogRead(0));

	//while (!serial);
	serial.println("running");
	serial.println();	serial.println();	serial.println();	serial.println();	serial.println();
	serial.print("UINT SIZE:");
	serial.println(sizeof(unsigned int));
	serial.print("ULONG SIZE:");
	serial.println(sizeof(unsigned long long));
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
	//startTimer(4000);
}

void loop() 
{
	//memset(timeInput,0,sizeof(timeInput)*sizeof(debugManInt));
	//memset(foundInput,0,sizeof(timeInput)*sizeof(debugManInt));
	//return;
	static int foo = 0;
    //delay(1000);

	/*if(foo >= 2)
		return;
	foo++;*/
  
	delay(750);
	serial.println(dDataCount);
	detachInterrupt(digitalPinToInterrupt(demodOut));
	if (dDataCount > 512)
	{
		for (int i = 0; i < dDataCount; i++)
		{
			uint8_t dByte = tDiffPinBuf[dReadIndex++];
			uint8_t pinR = 0x01 & dByte;
			uint8_t tClass = (0xF0 & dByte) >> 4;
			int ret = HandleIntManchester(pinR, tClass);
			if (ret > 0) // found packet
			{
				serial.println("FOUND PACKET");
				serial.println("READ");
				EM4100Data *xd = (EM4100Data*)gClientPacketBufWithParity;
				//look at parity rows
				for(int i=0;i<10;i++)
				{
					serial.print("[");
					serial.print(xd->lines[i].data_nibb,HEX);
					serial.print("] ");
					serial.print(xd->lines[i].data_nibb,BIN);
					serial.print(", ");
					serial.print(xd->lines[i].parity);
					serial.print(", ");
					serial.println(has_even_parity(xd->lines[i].data_nibb,4));
				}
				gPacketRead = 0;
			}
			if (dReadIndex >= nBitRingBufLength)
			dReadIndex = 0;
		}
		dDataCount = 0;
	}
	attachInterrupt(digitalPinToInterrupt(demodOut), INT_manchesterDecode, CHANGE);
	return;
	//startTimer(3800);
	//transmit(gFakeData,sizeof(gFakeData),noiseBits,sizeof(noiseBits));
	//gPacketRead = 0;
	//transmit(gFakeData,sizeof(gFakeData),noiseBits,sizeof(noiseBits));
	//detachInterrupt(digitalPinToInterrupt(demodOut));

	//delay(750);
	//digitalWrite(pLED,0);

	/*if(gHeaderRead)
	{
		for(int i=0;i<tbufsize;i++)
		{
			delay(10);
			serial.print("#:");
			serial.print((int)i);
			serial.print("\tdT: \t");
			serial.print(foundInput[i].thisTime);
			serial.print("\tI: \t");
			serial.print((int)foundInput[i].bitInput);
			serial.print("\tC: \t");
			serial.print((int)foundInput[i].timeClass);
			serial.print("\tR: \t");
			if(foundInput[i].bitRead == -1)
				serial.print("-");
			else
				serial.print((int)foundInput[i].bitRead);
			serial.print("\tHC: \t");
			serial.print((int)foundInput[i].headerCount);
			serial.print("\tHF: \t");
			serial.print((int)foundInput[i].headerFound);
			serial.print("\tSS: \t");
			serial.print((int)foundInput[i].syncState);
			serial.println();
		}
		delay(500);
		//gHeaderRead=0;
	}*/
	//delay(750);
	if(gPacketRead == 0)
	{
		//return;
		//serial.print("No new data ");
		//serial.print(" I ");
		//serial.print(gIntCount);
		//serial.print(" maxH ");
		//serial.println(gMaxHeaderFound);
		//attachInterrupt(digitalPinToInterrupt(demodOut), INT_manchesterDecode, CHANGE);

		//startTimer(3800);
		//delay(500);
		return;
	}
	//return;
	//serial.print("I: ");
	//serial.print(gIntCount);
	/*serial.print(" W: ");
	serial.print(gRingBufWrite);
	serial.print(" R: ");
	serial.print(gRingBufRead);*/
	/*serial.print(" B: ");
	serial.print(gReadBitCount);
	serial.println();
	uint32_t errors = 0;*/
	/*for(uint32_t i=0;i<(55);i++)//gReadBitCount<70 ? gReadBitCount : 70
	{
		unsigned short newbyte = ReadFromRB();
		int fakeme = (int)gFakeData[i];
		//if(i >= 9 && i < MANCHESTER_PACKET_SIZE + 9)
		//	fakeme = (int)gFakeData[i-9];
		//else if(i >= MANCHESTER_PACKET_SIZE + 9)
		//	fakeme = -1;
		//int fakeme = (int)gFakeData[i];
		serial.print(i);
		serial.print(", ");
		serial.print(newbyte);
		serial.print(", ");
		serial.print(fakeme);
		if(newbyte != fakeme)
		{   
			Serial.print(", X");
			errors++;
		}
		serial.println();
	}
	serial.print("Read Bits: ");
	serial.println(gReadBitCount);
	serial.print("Bit errors: ");
	serial.println(errors);*/
	/*errors=0;
	for(uint32_t i=0;i<sizeof(gClientPacketBufWithParity);i++)
	{
		serial.print(0b00011111 & (uint32_t)gClientPacketBufWithParity[i],BIN);
		serial.print(", ");
		serial.println((uint32_t)iFakeData[i],BIN);
	}*/
	serial.println("READ");
	EM4100Data *xd = (EM4100Data*)gClientPacketBufWithParity;
	//look at parity rows
	for(int i=0;i<10;i++)
	{
		serial.print(xd->lines[i].data_nibb,BIN);
		serial.print(", ");
		serial.print(xd->lines[i].parity);
		serial.print(", ");
		serial.println(!has_even_parity(xd->lines[i].data_nibb,4));
	}
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
	gPacketRead	=	0;
	//attachInterrupt(digitalPinToInterrupt(demodOut), INT_manchesterDecode, CHANGE);

	delay(200);
	return;
}