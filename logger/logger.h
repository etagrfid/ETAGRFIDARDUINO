#ifndef logger_h //load header if we have not defined RFIDuino_h
#define logger_h //define RFIDuino_h, to prevent code being loaded if this library is loaded a second time
#include "Arduino.h"
#define DELAYVAL    320   //384 //standard delay for manchster decode
#define TIMEOUT     1000  //standard timeout for manchester decode

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

//rtc
#define PCF8523_ADDRESS       0x68
#define PCF8523_CLKOUTCONTROL 0x0F
#define PCF8523_CONTROL_3     0x02

enum Pcf8523SqwPinMode { PCF8523_OFF = 7, PCF8523_SquareWave1HZ = 6, PCF8523_SquareWave32HZ = 5, PCF8523_SquareWave1kHz = 4, PCF8523_SquareWave4kHz = 3, PCF8523_SquareWave8kHz = 2, PCF8523_SquareWave16kHz = 1, PCF8523_SquareWave32kHz = 0 };

class clock {
public:
    boolean begin(void);
    void adjust(const DateTime& dt);
    boolean initialized(void);
    static DateTime now();

    Pcf8523SqwPinMode readSqwPinMode();
    void writeSqwPinMode(Pcf8523SqwPinMode mode);
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

//
//LOGGER
//
class logger{
	public:

		logger();
		//
		//void boot();
		//low level deocde functions
		bool decodeTag(unsigned char *buf);	//low level tag decode
		void transferToBuffer(byte *tagData, byte *tagDataBuffer);	//transfer data from one array to another
		bool compareTagData(byte *tagData1, byte *tagData2);			//compate 2 arrays

		//higher level tag scanning / reporting
		bool scanForTag(byte *tagData);
		void load_settings();
		void set_setting(String setting, String value);
		void boot();
		int get_RFID_READ_FREQ(){ 
			return RFID_READ_FREQ;
		}
		void capture_command();
		String time_stamp_string();
		//user output pin values, variables as they will change depending on which hardware is used
	private:
		//pins to connect Arduino to the EM4095 chip, variables as they will change depending on which hardware is used
		int demodOut;
		int shd;
		int mod;
		int rdyClk;

		//real time clock
		clock rtc;

		//settings and their default values
		int BOOT_TO_MODE = 0;
		int CURRENT_MODE = 0;
		String POWER_SCHEDULE = "00:00-24:00";
		bool JSON_RECORD = true;
		bool CSV_RECORD = true;
		int RFID_READ_FREQ = 200;
		int LOW_BATTERY = 500;
		int LONGITUDE = -80.60301463292694;
		int LATITUDE = 28.60739886098215;
		String FILE_PREFIX = "datalog";
		int HOURS_PER_FILE = 24;
		int COMMAND_TIMEOUT = 500;
};
#endif