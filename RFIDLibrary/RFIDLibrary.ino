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
#include <Arduino.h>
#include "ManchesterDecoder.h"

#define LED_PIN 13



//ETAG BOARD
/*#define serial SerialUSB
#define ShutdownPin 8
#define demodOut 30
ManchesterDecoder gManDecoder(demodOut,ShutdownPin,ManchesterDecoder::U2270B);*/


#define serial Serial
#define ShutdownPin 7 //test board
#define demodOut 8 
ManchesterDecoder gManDecoder(demodOut,ShutdownPin,ManchesterDecoder::EM4095);

void AttemptRFIDReading();
void TCconfig();
void sleep();

int gTC3_flag = 0;

void sleep()
{
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    /*if (USB->DEVICE.FSMSTATUS.bit.FSMSTATE == USB_FSMSTATUS_FSMSTATE_SUSPEND_Val) 
    {

        USBDevice.detach();

        __WFI();

        USBDevice.attach();
        USB->DEVICE.CTRLB.bit.UPRSM = 0x01u;
        while (USB->DEVICE.CTRLB.bit.UPRSM);
    }
    else*/
      __WFI();
}

void TCconfig()
{
    const uint8_t GCLK_SRC = 4;

    SYSCTRL->XOSC32K.bit.RUNSTDBY = 1;

    GCLK->GENDIV.reg = GCLK_GENDIV_ID(GCLK_SRC) | GCLK_GENDIV_DIV(2);
    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN |
            GCLK_GENCTRL_SRC_XOSC32K |
            GCLK_GENCTRL_ID(GCLK_SRC) |
            GCLK_GENCTRL_RUNSTDBY;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    PM->APBCMASK.reg |= PM_APBCMASK_TC3;

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
            GCLK_CLKCTRL_GEN(GCLK_SRC) |
            GCLK_CLKCTRL_ID(GCM_TCC2_TC3);
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    TC3->COUNT8.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);

    TC3->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT8 |
            TC_CTRLA_RUNSTDBY |
            TC_CTRLA_PRESCALER_DIV256;
    while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);


    TC3->COUNT8.INTENSET.reg = TC_INTENSET_OVF;
    while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);

    TC3->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);

    NVIC_EnableIRQ(TC3_IRQn);
    NVIC_SetPriority(TC3_IRQn, 0x00);
}

//Interrupt Service Routine
void TC3_Handler()
{
    if (TC3->COUNT8.INTFLAG.bit.OVF) 
    {
        TC3->COUNT8.INTFLAG.bit.OVF = 1;
        gTC3_flag = true;
    }
}
void setup() 
{
  TCconfig();
	pinMode(PIN_LED,OUTPUT);
	digitalWrite(PIN_LED,LOW);
	serial.begin(115200);
	delay(500);
	serial.println("running");
  delay(2000);
  sleep();
}

void loop() 
{
  if (gTC3_flag) 
  {
    digitalWrite(LED_PIN,HIGH);
    delay(100);
    digitalWrite(LED_PIN,LOW);
    serial.begin(115200);
    delay(50);
    static int wakeups = 0;
    serial.print("wakeup ");
    serial.println(wakeups++);
    delay(50);
    gManDecoder.EnableMonitoring(); //re-enable the interrupt 
    delay(1000);
    AttemptRFIDReading();
    gTC3_flag = 0;
  }
  gManDecoder.DisableChip();
  sleep();
}
 void AttemptRFIDReading()
 { 
	serial.print("Check: ");
	serial.println(gManDecoder.GetBitIntCount());
	static int packetsFound = 0;
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
		//serial.println("FOUND PACKET");
		//serial.println("READ");
		//look at parity rows
    //use to look at binary card data
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
			/*serial.print(data0,HEX);
			if(i<8)
				serial.print(",");*/
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
		serial.println(cardID);
		serial.print("Card Number: ");
		serial.println(cardNumber);
		//serial.println();
		//serial.println(packetsFound++);
   //delay(100);
	}

}
