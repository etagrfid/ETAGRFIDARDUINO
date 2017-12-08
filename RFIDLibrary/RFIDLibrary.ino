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
 * RFIDLibrary.ino
 *
 * Created: 11/14/2017 7:26:06 PM
 *  Author: Jay Wilhelm jwilhelm@ohio.edu
 */ 


 //1 Read RFID
 //2 Sleep
 //3 Repeat 1
#include <Arduino.h>
#include "ManchesterDecoder.h"
#include <RTCZero.h>

RTCZero rtc;

//ETAG BOARD
#define serial SerialUSB
#define ShutdownPin 8
#define demodOut 30

/*#define serial Serial
#define ShutdownPin 7 //test board
#define demodOut 8 */

ManchesterDecoder gManDecoder(demodOut,ShutdownPin,ManchesterDecoder::U2270B);
void ISRWakeup(void){								//blink when chip wakes up
  digitalWrite(11, HIGH);
  //delayMicroseconds(100000);
  //digitalWrite(11,LOW);
}

void setup() 
{
	pinMode(11, OUTPUT);
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,HIGH);
	serial.begin(115200);
	serial.println("running");
	/*USBDevice.detach();
  delay(2000);
  rtc.begin();
  rtc.setTime(0,00,00);
  rtc.setDate(24,9,16);
  rtc.setAlarmTime(00,00,10);			//sleep for ten seconds
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(ISRWakeup);
  digitalWrite(ShutdownPin, LOW);        //Turn off RFID chip to reduce power
  rtc.standbyMode();						//Put chip to sleep
  //digitalWrite(ShutdownPin, HIGH);		//turn RFID chip back on
  USBDevice.init();      //Including this increases power by ~5mA during sleep mode
  USBDevice.attach();
  delay(2000);
  serial.begin(115200);
  serial.println("wakeup");
  gManDecoder.WakeupFromSleep();*/
  gManDecoder.EnableMonitoring();
}

void loop() 
{
  // for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
  // {
  //  pinMode( ul, INPUT ) ;
  // }

  
  serial.print("Check: ");
  serial.println(gManDecoder.GetBitIntCount());
	static int packetsFound = 0;
	delay(500);
  digitalWrite(PIN_LED,!digitalRead(PIN_LED));

	int p_ret = gManDecoder.CheckForPacket();//check if there is data in the interrupt buffer
	if(p_ret > 0)
	{
		EM4100Data xd; //special structure for our data
		int dec_ret = gManDecoder.DecodeAvailableData(&xd); //disable the interrupt and process available data
		if(dec_ret <= 0)
		{
			gManDecoder.EnableMonitoring();
			return;
		}
		for(int i=0;i<11;i++)
		{
			serial.print("[");
			serial.print(xd.lines[i].data_nibb,HEX);
			serial.print("] ");
			serial.print(xd.lines[i].data_nibb,BIN);
			serial.print(", ");
			serial.print(xd.lines[i].parity);
			serial.print(", ");
			serial.println(has_even_parity(xd.lines[i].data_nibb,4));
		}
		uint8_t cardID = 0;
		uint32_t cardNumber = 0;
		//serial.print("Data: ");
		for(int i=0;i<10;i+=2)
		{
			uint8_t data0 = (xd.lines[i].data_nibb << 4) | xd.lines[i+1].data_nibb;
			//use to look at hex card data
			
			if(i<2)
			{
				cardID = data0;
			}
			else
			{
				cardNumber <<= 8;
				cardNumber |= data0;
			}
		}  
		serial.println();
		serial.print("Card ID: ");
		digitalWrite(11, HIGH); //blink led on 11 when tag is read
		delay(50);
		digitalWrite(11,LOW);
		delay(50);
		digitalWrite(11, HIGH);
		delay(50);
		digitalWrite(11,LOW);
		delay(50);
		digitalWrite(11, HIGH);
		delay(50);
		digitalWrite(11,LOW);	
		serial.println(cardID);
		serial.print("Card Number: ");
		serial.println(cardNumber);
		serial.println();
		serial.println(packetsFound++); 
	}
  gManDecoder.EnableMonitoring(); //re-enable the interrupt 
  //USBDevice.detach();
  // for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
  // {
  //  pinMode( ul, OUTPUT );
  // }
}
