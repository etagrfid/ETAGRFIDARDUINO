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


#define pLED 13

//#define EM4095
//ETAG BOARD
#define serial SerialUSB
#define ShutdownPin 8
#define demodOut 30

/*#define serial Serial
#define ShutdownPin 7 //test board
#define demodOut 8 */

#define STANDARD_BUFFER_FILL_TIME 100 //tweak the dwell time (ms)

#define CollectedBitMinCount 120 //used to tweak read speed, reduces chances of good read
ManchesterDecoder gManDecoder(demodOut,ShutdownPin,ManchesterDecoder::EM4095,CollectedBitMinCount);

void setup() 
{
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,HIGH);
  
	serial.begin(115200);
	serial.println("running");
	gManDecoder.EnableMonitoring();
}
void PrintTagID(EM4100Data &xd)
{
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
}
void loop() 
{  
  //gManDecoder.DisableMonitoring();
  //gManDecoder.ChipOff();
  //delay(1500);//simulate sleeping
  gManDecoder.ChipOn();
  gManDecoder.EnableMonitoring(); //re-enable the interrupt
  delay(STANDARD_BUFFER_FILL_TIME);
  //digitalWrite(PIN_LED,!digitalRead(PIN_LED));
  serial.print("Check: ");
  serial.println(gManDecoder.GetBitIntCount());
	int p_ret = gManDecoder.CheckForPacket();//check if there is data in the interrupt buffer
	if(p_ret == 0)
  {
    gManDecoder.EnableMonitoring(); //re-enable the interrupt
    delay(STANDARD_BUFFER_FILL_TIME);
    p_ret = gManDecoder.CheckForPacket();
    serial.println("Second Check");
  }
	
	if(p_ret > 0)
	{
		EM4100Data xd; //special structure for our data
		int dec_ret = gManDecoder.DecodeAvailableData(&xd); //disable the interrupt and process available data
		if(dec_ret == DECODE_PARITY_FAILED)
    {
      serial.println("!Packet found, but failed parity");
      PrintTagID(xd);
    }
    else if(dec_ret != DECODE_FOUND_GOOD_PACKET)
    {
      serial.println("!Packet not found");
      return;
    }
    else{
      serial.println("!Packet found good");
      //PrintTagID(xd);
      String bintag = gManDecoder.GetFullBinaryString(xd);
      String hextag = gManDecoder.GetDecodedHexNumberAsString(xd);
      serial.println(hextag);
      uint32_t tagNum = gManDecoder.GetTagNumber(xd);
      serial.println(tagNum);
      uint8_t cardID = gManDecoder.GetCardIDNumber(xd);
      serial.println(cardID);
    }
	}

}
