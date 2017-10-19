#define num_bits 10
#define outputpin 4
bool data[num_bits];

void sendBit(int mics, bool data) {
  int prev_time = micros();
  if (data) {
    digitalWrite(outputpin, HIGH);
    while (mics + prev_time > micros());
    prev_time = micros();
    digitalWrite(outputpin, LOW);
    while (mics + prev_time > micros());
    return;
  } else {
    digitalWrite(outputpin, LOW);
    while (mics + prev_time > micros());
    prev_time = micros();
    digitalWrite(outputpin, HIGH);
    while (mics + prev_time > micros());
    return;
  }
}

void transmit(bool data[]) {
  for (int i = 0; i < num_bits; i++) {
    sendBit(250, data);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(outputpin, OUTPUT);
  for (int i = 0; i < num_bits; i++) {
    data[i] = true;//(0 == i % 2);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  transmit(data);
  delay(1000);
}
