#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  load_settings();
}

void loop() {
  // nothing happens after setup
}

void load_settings() {
  char current_setting[30] = "";
  char value[30] = "";
  int next_index = 0;
  int value_index = 0;
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  File settings = SD.open("settings.txt");
  if (settings) {
    char letter;
    while (settings.available()) {
      letter = settings.read();
      switch (letter) {
        case '#':
          while (settings.available() && letter != '\n' && letter != '\r') {
            letter = settings.read();
          }
          break;
        case '\t':
        case '\n':
        case '\r':
        case ' ': //do nothing with spaces new lines and tabs
          break;
        case ':':
          while (settings.available() && value[value_index] != ';') { //read in setting value
            value[value_index] = settings.read();
            value_index++;
          }

          if(current_setting == "DEFAULT_MODE"){ //figure out what the setting is
            Serial.println("boot mode read");
          }else if(current_setting == "POWER_SCHEDULE"){
            Serial.println("power schedule read");
          }else{
            Serial.println(current_setting);
          }

          while (value_index > 0) {  //clear value for next use
            value[value_index] = '\0';
            value_index--;
          }

          while (next_index > 0) {  //clear setting for next use
            current_setting[next_index] = '\0';
            next_index--;
          }
          next_index = 0; //reset index
          break;
        default:
          current_setting[next_index] = letter;
          next_index++;
          break;
      }
    }
    settings.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }


}
//test