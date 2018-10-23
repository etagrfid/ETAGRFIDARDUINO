/****************************************************************************
 *
 *   Copyright (c) 2017 Jay Wilhelm. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name ETAGRFIDArduino nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/*
 * ETAG_LowPowerTest.ino
 *
 * Created: 10/1/2018 3:16:26 PM
 *  Author: Jay Wilhelm jwilhelm@ohio.edu
 *	Includes low power testing
 */ 

#include <Arduino.h>
#include <RTCZero.h>

/* Create an rtc object */
RTCZero rtc;
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */
const byte day = 25;
const byte month = 9;
const byte year = 15;

//#define LED_BUILTIN 31	//48 for the xplained D21
#define BUTTON_PIN	3	//5 for the xplained D21
void ding()
{
	volatile int x = 43;
	x++;
}
void alarmMatch()
{
	volatile int x = 43;
	x++;
	//Serial.println("Alarm Match!");
}
void LowPower_SetUSBMode(void)
{
	//begin USB disable for low power
	USBDevice.detach();
	USBDevice.standby();
	USB->DEVICE.CTRLA.bit.RUNSTDBY = 0;
	PM->APBBMASK.reg &= ~PM_APBBMASK_USB;
	
	USB->DEVICE.CTRLA.bit.SWRST = 1;	//reset
	USB->DEVICE.CTRLA.bit.ENABLE = 0;	//disable
	USB->DEVICE.CTRLB.bit.UPRSM = 0;	//nowake host
	USB->DEVICE.CTRLB.bit.SPDCONF = 0;
	#ifdef PIN_LED_TXL
	pinMode(PIN_LED_TXL, INPUT);
	pinMode(PIN_LED_RXL, INPUT);
	#endif
	PORT->Group[0].PINCFG[PIN_PA24G_USB_DM].bit.PMUXEN = 0;
	PORT->Group[0].PINCFG[PIN_PA25G_USB_DP].bit.PMUXEN = 0;
	//end USB disable
}
void SetRTC_AlarmDelta(void)
{
	rtc.setTime(hours, minutes, seconds);
	rtc.setDate(day, month, year);

	rtc.setAlarmTime(16, 0, 10);
	rtc.enableAlarm(rtc.MATCH_HHMMSS);
}

void LowPower_SetGPIO(void)
{
	//PA21 SD_CS + RTC INT (ext pullup) (input)
	//PA22 SCL (ext pullup) (input)
	//PA23 SDA (ext pullup) (input)
	pinMode(7,INPUT);
	pinMode(16,INPUT);
	pinMode(17,INPUT);

	//PA08 Flash CS
	//PB10 MOSI (SD, Flash)
	//PB11 SCK (SD, Flash)
	//PA12 MISO (SD, Flash)
	//PA28 SD FET (low)
	pinMode(2,INPUT);
	pinMode(21,OUTPUT);	//THIS ONE CAUSES HIGH POWER IF INPUT
	digitalWrite(21,LOW);
	pinMode(20,OUTPUT);digitalWrite(20,HIGH);//THIS ONE CAUSES HIGH POWER IF INPUT
	pinMode(18,INPUT);
	pinMode(32,INPUT);

	//PA27 LED
	pinMode(LED_BUILTIN,OUTPUT);
	digitalWrite(LED_BUILTIN,HIGH);
	

	//PB03 RFID Read
	//PA07 RFID Shutdown #2
	//PA06 RFID Shutdown #1
	pinMode(30,OUTPUT);digitalWrite(30,LOW);//THIS ONE CAUSES HIGH POWER IF INPUT
	//return; //915 to 866uA here
	
	
	pinMode(46,INPUT);
	pinMode(8,INPUT);
	//The following change from INPUT to OUTPUT causes the RFID chip to pulse on/off at 100 mA

	/*pinMode(46,OUTPUT);
	pinMode(8,OUTPUT);
	digitalWrite(46,HIGH);//EM4095 HIGH = off, LOW = on, but at increased power
	digitalWrite(8,HIGH);*/

	
	//PA02 Battery read
	pinMode(24,INPUT);

	
	//PA24 USB+	
	//PA25 USB-
	//powered from battery -> ~7mA when active
	//powered from battery -> 750 uA (PowerDebugger)
	//powered from USB -> 580 uA (PowerDebugger)
	//powered from USB -> 11 mA (PowerDebugger)
}

// the setup function runs once when you press reset or power the board
void setup()
{
	LowPower_SetUSBMode(); //has to be here or something else upsets low power
	
	
	// initialize digital pin LED_BUILTIN as an output.
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	//pinMode(BUTTON_PIN,INPUT_PULLUP);
	pinMode(BUTTON_PIN,INPUT);
	
	attachInterrupt(BUTTON_PIN, ding, RISING);
	
	rtc.begin(); // initialize RTC 24H format

	SetRTC_AlarmDelta();
	
	rtc.attachInterrupt(alarmMatch);
	
	
	// Set the XOSC32K to run in standby
	SYSCTRL->XOSC32K.bit.RUNSTDBY = 1;
	
	// Configure EIC to use GCLK1 which uses XOSC32K
	// This has to be done after the first call to attachInterrupt()
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_EIC) |
	GCLK_CLKCTRL_GEN_GCLK1 |
	GCLK_CLKCTRL_CLKEN;		
}
#define GENERIC_CLOCK_GENERATOR_MAIN      (0u)

// the loop function runs over and over again forever
void loop()
{
	LowPower_SetGPIO();
	GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_MAIN ) ; // Generic Clock Generator 0

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
		/* Wait for synchronization */
	}

	/* Write Generic Clock Generator 0 configuration */
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_MAIN ) | // Generic Clock Generator 0
	GCLK_GENCTRL_SRC_OSCULP32K | 
	//GCLK_GENCTRL_SRC_OSC8M |
	//GCLK_GENCTRL_SRC_XOSC32K |
	//GCLK_GENCTRL_SRC_DFLL48M | // Selected source is DFLL 48MHz
	//                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
	GCLK_GENCTRL_IDC | // Set 50/50 duty cycle
	GCLK_GENCTRL_GENEN ;

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
		/* Wait for synchronization */
	}
	
	//turn off the DFLL?
	SYSCTRL->DFLLCTRL.reg = 0x00;//SYSCTRL_DFLLCTRL_MODE | /* Enable the closed loop mode */
	//SYSCTRL_DFLLCTRL_WAITLOCK |
	//SYSCTRL_DFLLCTRL_QLDIS ; /* Disable Quick lock */
	SYSCTRL->DFLLCTRL.reg &= ~SYSCTRL_DFLLCTRL_ENABLE ;
	
	//digitalWrite(LED_BUILTIN,digitalRead(5));
	SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk;
	
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	__DSB();
	__WFI();
	
	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
	 /* Enable the DFLL */
	 SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_MODE | /* Enable the closed loop mode */
	 SYSCTRL_DFLLCTRL_WAITLOCK |
	 SYSCTRL_DFLLCTRL_QLDIS ; /* Disable Quick lock */
	   while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
	   {
		   /* Wait for synchronization */
	   }
	 SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE ;

	 while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 ||
	 (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0 )
	 {
		 /* Wait for locks flags */
	 }


	 while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
	 {
		 /* Wait for synchronization */
	 }
	 
	GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_MAIN ) ; // Generic Clock Generator 0

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
		/* Wait for synchronization */
	}

	/* Write Generic Clock Generator 0 configuration */
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_MAIN ) | // Generic Clock Generator 0
	//GCLK_GENCTRL_SRC_OSC8M |
	//GCLK_GENCTRL_SRC_XOSC32K |
	GCLK_GENCTRL_SRC_DFLL48M | // Selected source is DFLL 48MHz
	//                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
	GCLK_GENCTRL_IDC | // Set 50/50 duty cycle
	GCLK_GENCTRL_GENEN ;

	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
	{
		/* Wait for synchronization */
	}
	
	
	
	SysTick->CTRL  |= SysTick_CTRL_ENABLE_Msk;




	delay(50);
	USBDevice.init();
	USBDevice.attach();
	
	int count = 0;
	while(1)
	{
		digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
		delay(250);                       // wait for a second
		digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
		delay(250);                       // wait for a second
		SerialUSB.print("Hello ");
		SerialUSB.println(count);
		if(count++ > 25)
			break;
	}
	SetRTC_AlarmDelta();
	LowPower_SetUSBMode();
}
