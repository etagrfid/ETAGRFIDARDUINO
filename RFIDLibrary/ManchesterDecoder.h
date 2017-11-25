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




void dprintf(char *fmt, ... );

void INT_manchesterDecode(void);
int has_even_parity(uint16_t x,int datasize);
int CheckManchesterParity(EM4100Data *xd);

class ManchesterDecoder
{
public:
	#define zShortLow 150
	#define zShortHigh 350

	#define zLongLow 400
	#define zLongHigh 600
	int gFoundPackets = 0;
	volatile uint8_t    gClientPacketBufWithParity[11];
	volatile uint8_t	gPacketRead = 0;


public:
	uint8_t mPIN_demodout	=	8;
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
	ManchesterDecoder(uint8_t demodPin);
	void ResetMachine();
	int UpdateMachine(int8_t currPin, uint32_t currTime,int8_t timeClass);
	void EnableMonitoring(void);
	int CheckForPacket(void);	
	int DecodeAvailableData(EM4100Data *bufout);
	int UpdateMachineUsingClass(int8_t currPin, int8_t timeClass);
	int StoreNewBit(int8_t newB);
	int HandleIntManchester(int8_t fVal, int8_t fTimeClass);
	
	enum TimeClass { tUnknown = 0,
		tShort,
	tLong};
};


#endif /* MANCHESTERINCLUDE_H_ */