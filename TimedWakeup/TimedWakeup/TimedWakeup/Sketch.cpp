/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */

/*
  TimedWakeup

  This sketch demonstrates the usage of Internal Interrupts to wakeup a chip in sleep mode.
  Sleep modes allow a significant drop in the power usage of a board while it does nothing waiting for an event to happen. Battery powered application can take advantage of these modes to enhance battery life significantly.

  In this sketch, the internal RTC will wake up the processor every 2 seconds.
  Please note that, if the processor is sleeping, a new sketch can't be uploaded. To overcome this, manually reset the board (usually with a single or double tap to the RESET button)

  This example code is in the public domain.
*/

#include "ArduinoLowPower.h"
#include <RTCZero.h>
ArduinoLowPowerClass LowPower;

RTCZero rtc;


//Beginning of Auto generated function prototypes by Atmel Studio
void dummy();
//End of Auto generated function prototypes by Atmel Studio
void ISR(){

analogWrite(12,255);
delayMicroseconds(500000);
analogWrite(12,0);
delayMicroseconds(3000000);




}
void extInterrupt(int interruptPin) {
pinMode(interruptPin, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(interruptPin), ISR, LOW);
}





#define interruptpin 3

void setup() {
/*pinMode(1,INPUT);  // tested whether setting pinmodes increased power,
pinMode(2,INPUT);	//  it did not
pinMode(4,INPUT);
pinMode(5,INPUT);
pinMode(6,INPUT);
pinMode(7,INPUT);
pinMode(8,INPUT);
pinMode(9,INPUT);
pinMode(10,INPUT);
pinMode(11,INPUT);
pinMode(13,INPUT); */
  delay(2000);
 // Serial.begin(115200);
  rtc.begin();
  rtc.setTime(0,00,00);
  rtc.setDate(24,9,16);
 //rtc.setAlarmTime(00,00,3;
  //rtc.enableAlarm(rtc.MATCH_HHMMSS);
 // rtc.attachInterrupt(ISR);
 extInterrupt(interruptpin);

  //rtc.standbyMode();


  //pinMode(LED_BUILTIN, OUTPUT);
  
  // Uncomment this function if you wish to attach function dummy when RTC wakes up the chip
  // LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, dummy, CHANGE);
}

void loop() {
rtc.standbyMode();
/*static int altime = 3;           this line allows the cpu to go to sleep and wake up at set intervals
rtc.setAlarmTime(00,00,altime);
rtc.standbyMode();
analogWrite(12,255);
delay(500);
analogWrite(12,0);
altime = altime+3;
delay(3000); */

  /*delay(5000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500); */
  // Triggers a 2000 ms sleep (the device will be woken up only by the registered wakeup sources and by internal RTC)
  // The power consumption of the chip will drop consistently
  //LowPower.sleep(2000);
}

void dummy() {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
}
