Low Power Methods: 


rtc.standbyMode  (Sketch.cpp)
- runs SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  __WFI(); turns off the CPU and waits for interrupt
- attachInterrupt(digitalPinToInterrupt(interruptPin), ISR, LOW);
	- ISR runs during interrupt

disable USB device (main.cpp)
- comment out:
	//USBDevice.init();			
	//USBDevice.attach();
- reduces current by 5mA, disables serial communication


Disable default Input buffer (wiring.c)
- comment out:
	// for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
 
	// {

 	//  pinMode( ul, INPUT ) ;

	// }
- prevents all pins from defaulting to input mode, causes no issues because using PinMode does this anyway