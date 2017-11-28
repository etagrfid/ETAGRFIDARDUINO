/*
 * ManchesterUtil.cpp
 *
 * Created: 11/14/2017 7:25:36 PM
 *  Author: Jay
 */ 
#include <Arduino.h>

#define NIBBLE_TO_BINARY_PATTERN "%c%c%c%c"
#define NIBBLE_TO_BINARY(byte)  \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

#include "ManchesterDecoder.h"
#define debug Serial
#include <stdarg.h>
void dprintf(char *fmt, ... ){
	char buf[128]; // resulting string limited to 128 chars
	va_list args;
	va_start (args, fmt );
	vsnprintf(buf, 128, fmt, args);
	va_end (args);
	debug.print(buf);
}
//#define printf dprintf

#define nBitRingBufLength 1024
volatile static uint8_t		tDiffPinBuf[nBitRingBufLength];
volatile static uint16_t	dWriteIndex = 0;
volatile static uint16_t	dReadIndex = 0;
volatile static uint16_t	dDataCount = 0;
volatile static uint8_t		gPIN_demodout	=	8;
void INT_manchesterDecode(void)
{
	volatile uint32_t timeNow = micros();
	volatile static uint32_t lastTime = 0;
	volatile static uint8_t  lastRead = 0;
	uint32_t fDiff = timeNow - lastTime;
	lastTime = timeNow;
	int8_t fTimeClass = ManchesterDecoder::tUnknown;
	int8_t fVal = !digitalRead(gPIN_demodout);
	
	if(fVal != lastRead)
	{
		if (fDiff >= zShortLow && fDiff <= zShortHigh)
			fTimeClass = ManchesterDecoder::tShort;
		else if (fDiff >= zLongLow && fDiff <= zLongHigh)
			fTimeClass = ManchesterDecoder::tLong;
		
		if (fDiff > 15 && fTimeClass != ManchesterDecoder::tUnknown)
		{
			tDiffPinBuf[dWriteIndex] = 0;
			tDiffPinBuf[dWriteIndex] = 0xF0 & (fTimeClass << 4);
			tDiffPinBuf[dWriteIndex] |= 0x0F & fVal;
			dWriteIndex++;
			if (dWriteIndex >= nBitRingBufLength)
				dWriteIndex = 0;
			dDataCount++;
		}
		else
			volatile int x = 44;//debug only
	}
	lastRead = fVal;
}


int has_even_parity(uint16_t x,int datasize)
{
	volatile unsigned int count = 0, i, b = 1;

	for(volatile int i = 0; i < datasize; i++)
	{
		if( x & (b << i) )
		{
			count++;
		}
	}

	if( (count % 2)==0 )
	{
		return 0;
	}
	
	return 1;
}

int CheckManchesterParity(EM4100Data *xd)
{
	volatile int row_err_count=0;
	volatile int col_err_count=0;
	volatile int err_count = 0;
	for(volatile int i=0;i<10;i++)
	{
		volatile bool this_row = has_even_parity(xd->lines[i].data_nibb,4);
		if(this_row != xd->lines[i].parity)
			row_err_count++;
	}
	for(volatile int i=0;i<4;i++)
	{
		volatile uint16_t coldata = 0x00;
		volatile uint16_t imask = 0x01 << i;
		for(int ii=0;ii<10;ii++)
		{
			uint16_t thisbit = xd->lines[ii].data_nibb;
			thisbit &= imask;
			thisbit >>= i;
			coldata |= thisbit  << ii;
		}
		
		volatile bool this_colp = has_even_parity(coldata,10);
		volatile uint8_t readparity = xd->colparity;
		volatile uint8_t read_col_par_bit = (readparity & imask) >> i;
		if(this_colp != read_col_par_bit)
			col_err_count++;
	}
	err_count = col_err_count+row_err_count;
	return err_count;
}


ManchesterDecoder::ManchesterDecoder(uint8_t demodPin) {
	mPIN_demodout = demodPin;
	pinMode(mPIN_demodout,INPUT);
	gPIN_demodout = mPIN_demodout;
	intCount = 0;
	lastTime = 0;
	secondLastTime = 0;
	lastValue = -1;
	secondLastValue = -1;
	ResetMachine();
}
void ManchesterDecoder::ResetMachine()
{
	headerFound = 0;
	headerCount = 0;
	syncState = 0;
	
	//data saving
	dataBinWrite = 0;
	dataBufWrite = 0;
	dataBinCount = 0;
	memset((uint8_t*)dataBuf,0,sizeof(dataBuf));

	//dataBuf[0] = 0x00;
}
int ManchesterDecoder::UpdateMachine(int8_t currPin, uint32_t currTime,int8_t timeClass)
{
	//secondLastTimeClass = lastTimeClass;
	lastTimeClass = timeClass;

	secondLastValue = lastValue;
	lastValue = currPin;

	secondLastTime = lastTime;
	lastTime = currTime;

	intCount++;
	return 0;
}
void ManchesterDecoder::EnableMonitoring(void)
{
	attachInterrupt(digitalPinToInterrupt(mPIN_demodout), INT_manchesterDecode, CHANGE);
}
int ManchesterDecoder::CheckForPacket(void)
{
	if (dDataCount >= 512)
	{
		//detachInterrupt(digitalPinToInterrupt(mPIN_demodout));
		return	1;
	}
	return 0;		
}
int ManchesterDecoder::DecodeAvailableData(EM4100Data *bufout)
{
	if (dDataCount < 512)
		return -1;
	detachInterrupt(digitalPinToInterrupt(mPIN_demodout));
	//gPacketRead = 0;
	for (int i = 0; i < 512; i++)
	{
		uint8_t dByte = tDiffPinBuf[dReadIndex++];
		if (dReadIndex >= nBitRingBufLength)
			dReadIndex = 0;
		uint8_t pinR = 0x01 & dByte;
		uint8_t tClass = (0xF0 & dByte) >> 4;
		printf("%d ",i);
		int ret = HandleIntManchester(pinR, tClass);
		if (ret > 0) // found packet
		{
			EM4100Data *testData = (EM4100Data*)this->gClientPacketBufWithParity;
			volatile int pcheck = CheckManchesterParity(testData);
			//gPacketRead = 0;
			if(pcheck == 0)
			{
				int dsize = sizeof(EM4100Data);
				memset(bufout,0x00,sizeof(EM4100Data));
				memcpy(bufout,(EM4100Data*)this->gClientPacketBufWithParity,sizeof(EM4100Data));//gClientPacketBufWithParity,sizeof(EM4100Data));
				ResetMachine();
				memset((uint8_t*)tDiffPinBuf,0,nBitRingBufLength);
				//globals
				dReadIndex = dWriteIndex;
				//dReadIndex = 0;
				dDataCount = 0;
				
				return 1;
			}
			else
			{
				for(int i=0;i<10;i++)
				{
					printf("[%X] "NIBBLE_TO_BINARY_PATTERN", %d, %d\n", testData->lines[i].data_nibb,NIBBLE_TO_BINARY(testData->lines[i].data_nibb),testData->lines[i].parity,has_even_parity(testData->lines[i].data_nibb,4));
					/*debug.print("[");
					debug.print(testData->lines[i].data_nibb,HEX);
					debug.print("] ");
					debug.print(testData->lines[i].data_nibb,BIN);
					debug.print(", ");	
					debug.print(testData->lines[i].parity);
					debug.print(", ");
					debug.println(has_even_parity(testData->lines[i].data_nibb,4));*/
				}
				debug.print("{P} ");
				debug.print(testData->colparity,BIN);
				debug.print(", STOP: ");
				debug.println(testData->stop_bit);
				delay(100);
				debug.println("FAILED PARITY");
			}
			/*serial.println("FOUND PACKET");
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
			}*/
			
		}
		
	}
	dDataCount = 0;
	return 0;
}

int ManchesterDecoder::UpdateMachineUsingClass(int8_t currPin, int8_t timeClass)
{
	return UpdateMachine(currPin, 0,timeClass);
}
int ManchesterDecoder::StoreNewBit(int8_t newB)
{
	//dataBuf[dataBufWrite] |= newB << (4 - dataBinWrite++);
	dataBuf[dataBufWrite] = (dataBuf[dataBufWrite] << 1) | (0x01 & newB);
	//dataBuf[dataBufWrite] <<= 1;
	//dataBuf[dataBufWrite] |= 0x01 & newB;
	dataBinWrite++;
	//newB << (dataBinWrite++ % 5);
	if (dataBinWrite == 5)
	{
		dataBinWrite = 0;
		dataBufWrite++;
		if (dataBufWrite >= sizeof(dataBuf))
			dataBufWrite = 0;
	}
	dataBinCount++;
	return 0;
}


int ManchesterDecoder::HandleIntManchester(int8_t fVal, int8_t fTimeClass)
{
	if (fTimeClass != ManchesterDecoder::tLong && fTimeClass != ManchesterDecoder::tShort)
		return 0;

	printf("%i T - %d P %d SS %d ", this->intCount, fTimeClass, fVal, this->syncState);
	
	if (this->syncState == 0 &&
	fVal == 0 && this->lastValue == 1 && this->secondLastValue == 0 &&
	fTimeClass == ManchesterDecoder::tShort && this->lastTimeClass == ManchesterDecoder::tLong)
	{
		printf("Sync at %d", this->intCount);
		this->syncState = 1;
		this->headerCount = 1;
		this->headerFound = 0;
		//dataCap = [];
		//headerLocation = i;
		/////dprintf("\n");
	}
	if (this->syncState == 1 && fVal == 1 && this->secondLastValue == 1 && fTimeClass == ManchesterDecoder::tShort && this->lastTimeClass == ManchesterDecoder::tShort)
	{
		this->headerCount++;
		printf("Up Header at %d, %d", this->intCount, this->headerCount);
		if (this->headerCount == 9 && this->headerFound == 0)
		{
			printf(" Header at %d\n", this->intCount);
			this->headerFound = 1;
			this->syncState = 2;
			//this->headerLocation = i - 15;
			//return this->UpdateMachine(pinRead, timeNow, timeClass);
		}
		/////else
		/////	printf("\n");
	}
	else if(this->syncState == 1 && fTimeClass == ManchesterDecoder::tLong)
	{
		printf("Reset at %d\n", this->intCount);
		this->ResetMachine();
		return this->UpdateMachineUsingClass(fVal, fTimeClass);
	}
	if (this->syncState <= 1)
	{
		printf("\n");
		return this->UpdateMachineUsingClass(fVal, fTimeClass);
	}

	int newBit = 0;
	if (this->syncState == 2 && this->headerFound == 1 && fTimeClass == ManchesterDecoder::tShort)
	{
		this->syncState = 3;
		printf("%d Skip Short %d\n", this->intCount, fVal);
	}
	else if (this->syncState == 3 && this->headerFound == 1 && fTimeClass == ManchesterDecoder::tShort)
	{
		this->syncState = 2;
		printf("%d Keep Short %d - %d\n", this->intCount, fVal,this->dataBinCount);
		this->StoreNewBit(fVal);
		//dataCap = [dataCap; fVal];
		newBit = 1;
	}
	else if ((this->syncState == 2 || this->syncState == 3) && this->headerFound == 1 && fTimeClass == ManchesterDecoder::tLong)
	{
		this->syncState = 2;
		printf("%d Keep Long %d - %d\n", this->intCount, fVal,this->dataBinCount);
		//dataCap = [dataCap; fVal];
		this->StoreNewBit(fVal);
		newBit = 1;
	}
	else
		printf("Dead at %d\n", this->intCount);
		

	if ((this->dataBinCount % 5 == 0)  && this->dataBinCount > 0x00 && this->dataBinCount < 55 && newBit == 1)
	{
		uint8_t checkD = this->dataBuf[this->dataBufWrite-1];
		checkD >>= 1;//remove parity bit
		uint8_t pCalc = 0;
		for (int i = 0; i < 4; i++)
			pCalc += 0x01 & (checkD >> i);
			
		if ((pCalc % 2) != (0x01 & this->dataBuf[this->dataBufWrite-1]))
		{
			printf("Error at parity %d\n", this->intCount);
			this->ResetMachine();
		}
		else
			printf("Pass Parity\n");
		//(checkD >> 1) %2 != (0x01 & checkD)

	}

	if (this->dataBinCount >= 55)
	{
		//if(gPacketRead == 0)
		{
			memset((uint8_t*)gClientPacketBufWithParity,0x00,sizeof(gClientPacketBufWithParity));
			memcpy((uint8_t*)gClientPacketBufWithParity,(uint8_t*)this->dataBuf,sizeof(this->dataBuf));
			//gPacketRead = 1;
		}
		//EM4100Data *foo = (EM4100Data*)this->dataBuf;
		printf("End of packet at %d\n", this->intCount);
		this->ResetMachine();
		this->UpdateMachineUsingClass(fVal, fTimeClass);
		gFoundPackets++;
		
		//only for debug
		delay(100);
		return gFoundPackets;
	}

	return this->UpdateMachineUsingClass(fVal, fTimeClass);
}