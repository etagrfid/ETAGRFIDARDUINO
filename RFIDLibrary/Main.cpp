/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "ManchesterDecoder.h"


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

ManchesterDecoder gManDecoder(demodOut);


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

	serial.println("running");
	serial.println();	serial.println();	serial.println();	serial.println();	serial.println();
	serial.print("UINT SIZE:");
	serial.println(sizeof(unsigned int));
	serial.print("ULONG SIZE:");
	serial.println(sizeof(unsigned long long));
  
	delay(10);
	
	gManDecoder.EnableMonitoring();
	delay(10);
}

void loop() 
{  
	delay(750);
	detachInterrupt(digitalPinToInterrupt(demodOut));
	if(gManDecoder.CheckForPacket())
	{
		EM4100Data xd;
		gManDecoder.DecodeAvailableData(&xd);
		serial.println("FOUND PACKET");
		serial.println("READ");
		//look at parity rows
		for(int i=0;i<10;i++)
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
		serial.print("Data: ");
		for(int i=0;i<10;i+=2)
		{
			uint8_t data0 = (xd.lines[i].data_nibb << 4) | xd.lines[i+1].data_nibb;
			serial.print(data0,HEX);
			if(i<8)
				serial.print(",");
		}
		gManDecoder.EnableMonitoring();
	}
}