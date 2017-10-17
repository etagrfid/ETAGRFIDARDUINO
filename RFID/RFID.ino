#define serial SerialUSB
#define timeout_micro 1000

//pins for RFID chip
#define demodOut 30
#define shd 8

long previous_change = 0;
byte RFID_data[5];

void manchesterDecode() {
  if (previous_change == 0) {
    previous_change = micros();
    serial.println("1st change since boot")
    return;
  }
  int b = 0;
  for (; b < 8 && micros() < previous_change + timout_micro) {
    if(digitalRead(demodOut)){
      serial.println("HIGH")          
    }else{
      serial.println("LOW")
    }
  }
  if(b != 7){
    serial.println("incomplete byte: timeout");
  }
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

  attachInterrupt(demodOut, manchesterDecode, CHANGE)
}

void loop() {
}

