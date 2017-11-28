#define serial SerialUSB
#define timeout_micro 1000

//pins for RFID chip
//demodOut used to be 30 is now 5 for re-wire
#define demodOut 5
#define shd 8

long prev_time = 0;
long curr_time = 0;
byte RFID_data[5];
int num_stays = 0;
long time_diff;
bool header_detected = false;
int body_index = 0;
byte current_byte = 0;
int byte_index = 0;
const int num_data_bits = 40;
bool data_bool[num_data_bits];
int data_bool_index = 0;

void manchesterDecode() {
  serial.println("triggered");
  curr_time = micros();
  time_diff = curr_time - prev_time;
  prev_time = curr_time;
  if (!header_detected) {
    if (190 < time_diff && time_diff < 340) {
      num_stays++;
    } else if (400 < time_diff && time_diff < 550) {
      num_stays = 0;
    } else {
      num_stays = 0;
    }
    if (num_stays > 7) {
      serial.println("header detected");
      header_detected = true;
      num_stays = 0;
    }
  } else {
    if (190 < time_diff && time_diff < 340) {
      data_bool[data_bool_index] = true;
    } else if (400 < time_diff && time_diff < 550) {
      data_bool[data_bool_index] = false;
    } else {
      header_detected = false;
      data_bool_index = 0;
    }
    if (data_bool_index >= num_data_bits) {
      for (int i = 0; i < num_data_bits; i++) {
        serial.print(data_bool[i]);
      }
      data_bool_index = 0;
      header_detected = false;
    }
    data_bool_index++;
  }
}

void setup() {
  delay(5000);
  serial.begin(115200);
  while (!serial);
  serial.println("running");

  pinMode(shd, OUTPUT);
  pinMode(demodOut, INPUT);

  //set shd and MOD low to prepare for reading
  digitalWrite(shd, LOW);

  attachInterrupt(demodOut, manchesterDecode, CHANGE);
}

void loop() {
  while (true);
}


