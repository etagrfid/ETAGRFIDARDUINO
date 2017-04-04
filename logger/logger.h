#ifndef logger_h //load header if we have not defined RFIDuino_h
#define logger_h //define RFIDuino_h, to prevent code being loaded if this library is loaded a second time
#include "Arduino.h"
#define DELAYVAL    320   //384 //standard delay for manchster decode
#define TIMEOUT     1000  //standard timeout for manchester decode

class logger{
	public:

		logger();
			//i/o
		void errorSound(); 					//play an error sound to the buzzer
		void successSound(); 				//play a sucess sound ot the buzzer

		//low level deocde functions
		bool decodeTag(unsigned char *buf);	//low level tag decode
		void transferToBuffer(byte *tagData, byte *tagDataBuffer);	//transfer data from one array to another
		bool compareTagData(byte *tagData1, byte *tagData2);			//compate 2 arrays

		//higher level tag scanning / reporting
		bool scanForTag(byte *tagData);

		//user output pin values, variables as they will change depending on which hardware is used
private:
	//pins to connect Arduino to the EM4095 chip , variables as they will change depending on which hardware is used
	int demodOut;
	int shd;
	int mod;
	int rdyClk;
};
//
//CLOCK
//
class TimeSpan;

#define DS1307_ADDRESS  0x68
#define DS1307_CONTROL  0x07
#define DS1307_NVRAM    0x08

#define SECONDS_PER_DAY 86400L

#define SECONDS_FROM_1970_TO_2000 946684800



// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime {
	public:
	    DateTime (uint32_t t =0);
	    DateTime (uint16_t year, uint8_t month, uint8_t day,
	                uint8_t hour =0, uint8_t min =0, uint8_t sec =0);
	    DateTime (const DateTime& copy);
	    DateTime (const char* date, const char* time);
	    DateTime (const __FlashStringHelper* date, const __FlashStringHelper* time);
	    uint16_t year() const       { return 2000 + yOff; }
	    uint8_t month() const       { return m; }
	    uint8_t day() const         { return d; }
	    uint8_t hour() const        { return hh; }
	    uint8_t minute() const      { return mm; }
	    uint8_t second() const      { return ss; }
	    uint8_t dayOfTheWeek() const;

	    // 32-bit times as seconds since 1/1/2000
	    long secondstime() const;   
	    // 32-bit times as seconds since 1/1/1970
	    uint32_t unixtime(void) const;

	    DateTime operator+(const TimeSpan& span);
	    DateTime operator-(const TimeSpan& span);
	    TimeSpan operator-(const DateTime& right);

	protected:
	    uint8_t yOff, m, d, hh, mm, ss;
};

// Timespan which can represent changes in time with seconds accuracy.
class TimeSpan {
	public:
	    TimeSpan (int32_t seconds = 0);
	    TimeSpan (int16_t days, int8_t hours, int8_t minutes, int8_t seconds);
	    TimeSpan (const TimeSpan& copy);
	    int16_t days() const         { return _seconds / 86400L; }
	    int8_t  hours() const        { return _seconds / 3600 % 24; }
	    int8_t  minutes() const      { return _seconds / 60 % 60; }
	    int8_t  seconds() const      { return _seconds % 60; }
	    int32_t totalseconds() const { return _seconds; }
	    TimeSpan operator+(const TimeSpan& right);
	    TimeSpan operator-(const TimeSpan& right);
	protected:
    	int32_t _seconds;
};

enum clockPinMode { OFF = 0x00, ON = 0x80, SquareWave1HZ = 0x10, SquareWave4kHz = 0x11, SquareWave8kHz = 0x12, SquareWave32kHz = 0x13 };

class clock {
	public:
	    boolean begin(void);
	    static void adjust(const DateTime& dt);
	    uint8_t isrunning(void);
	    static DateTime now();
	    static clockPinMode readSqwPinMode();
	    static void writeSqwPinMode(clockPinMode mode);
	    uint8_t readnvram(uint8_t address);
	    void readnvram(uint8_t* buf, uint8_t size, uint8_t address);
	    void writenvram(uint8_t address, uint8_t data);
	    void writenvram(uint8_t address, uint8_t* buf, uint8_t size);
};


// RTC using the internal millis() clock, has to be initialized before use
// NOTE: this clock won't be correct once the millis() timer rolls over (>49d?)
class RTC_Millis {
	public:
    	static void begin(const DateTime& dt) { adjust(dt); }
    	static void adjust(const DateTime& dt);
    	static DateTime now();
	protected:
    	static long offset;
};
#endif
