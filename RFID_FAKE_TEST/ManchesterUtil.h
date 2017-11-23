/*
 * ManchesterInclude.h
 *
 * Created: 11/14/2017 7:26:06 PM
 *  Author: Jay
 */ 


#ifndef MANCHESTERINCLUDE_H_
#define MANCHESTERINCLUDE_H_

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

int has_even_parity(uint16_t x,int datasize);
int CheckManchesterParity(EM4100Data *xd);


class ManchesterIntInfo
{
	public:
	uint8_t headerFound = 0;
	uint8_t headerCount = 0;
	uint8_t syncState	= 0;
	uint32_t intCount = 0;

	uint32_t	lastTime;
	uint32_t	secondLastTime;
	int8_t		lastTimeClass;
	//int8_t		secondLastTimeClass;
	int8_t		lastValue;
	int8_t		secondLastValue;

	uint8_t		dataBuf[11];
	uint8_t		dataBinWrite;
	uint8_t		dataBufWrite;
	uint8_t		dataBinCount;
	public:
	ManchesterIntInfo() {
		intCount = 0;
		lastTime = 0;
		secondLastTime = 0;
		lastValue = -1;
		secondLastValue = -1;
		ResetMachine();
	};
	void ResetMachine()
	{
		headerFound = 0;
		headerCount = 0;
		syncState = 0;
		dataBinWrite = 0;
		dataBufWrite = 0;
		dataBinCount = 0;
		dataBuf[0] = 0x00;
	};
	int UpdateMachine(int8_t currPin, uint32_t currTime,int8_t timeClass)
	{
		//secondLastTimeClass = lastTimeClass;
		lastTimeClass = timeClass;

		secondLastValue = lastValue;
		lastValue = currPin;

		secondLastTime = lastTime;
		lastTime = currTime;

		intCount++;
		return 0;
	};
	int UpdateMachineUsingClass(int8_t currPin, int8_t timeClass)
	{
		return UpdateMachine(currPin, 0,timeClass);
	}
	int StoreNewBit(int8_t newB)
	{
		//dataBuf[dataBufWrite] |= newB << (4 - dataBinWrite++);
		dataBuf[dataBufWrite] <<= 1;

		dataBuf[dataBufWrite] |= newB;
		dataBinWrite++;
		//newB << (dataBinWrite++ % 5);
		if (dataBinWrite == 5)
		{
			dataBinWrite = 0;
			dataBufWrite++;
			if (dataBufWrite >= 12)
			dataBufWrite = 0;
			dataBuf[dataBufWrite] = 0x00;
		}
		dataBinCount++;
		return 0;
	}
	enum TimeClass { tUnknown = 0,
		tShort,
	tLong};
};


#endif /* MANCHESTERINCLUDE_H_ */