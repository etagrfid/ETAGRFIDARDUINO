#define serial SerialUSB
#define timeout_micro 1000

//pins for RFID chip
//demodOut used to be 30 is now 5 for re-wire
#define demodOut 5 
#define shd 8

long previous_change = 0;
byte RFID_data[5];

void manchesterDecode() {
  if(micros() < previous_change){
    return;
  }
  serial.println("Triggered");
  if (previous_change == 0) {
    previous_change = micros();
    serial.println("1st change since boot");
    return;
  }
  int b = 0;
  for (; b < 8 && micros() < previous_change + 8*timeout_micro; b++) {
    if(digitalRead(demodOut)){
      serial.println("HIGH");
    }else{
      serial.println("LOW");
    }
  }
  if(b != 8){
    serial.println("incomplete byte: timeout");
  }
  previous_change = previous_change + 100000;
}

void setup() {
  delay(10000);
  serial.begin(9600);
  while (!serial);
  serial.println("running");

  pinMode(shd, OUTPUT);
  pinMode(demodOut, INPUT);

  //set shd and MOD low to prepare for reading
  digitalWrite(shd, LOW);

  attachInterrupt(demodOut, manchesterDecode, CHANGE);
}

void loop() {
  serial.print(digitalRead(demodOut));
  int current = digitalRead(demodOut);
  while(current == digitalRead(demodOut));
  delay(1000);
}


