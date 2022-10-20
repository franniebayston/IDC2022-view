#include <SoftwareSerial.h>

#define TxPin 14

SoftwareSerial mySerial = SoftwareSerial(255, TxPin);

#define num 17

int durs[num]  = {211, 211, 211, 210, 210, 
                211, 211, 211, 211, 211, 211, 211, 210, 210, 211, 211, 212
};
int octs[num]  = {215, 215, 215, 215, 215, 
                215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215
};
int notes[num] = {220, 220, 220, 220, 220,
                220, 225, 232, 220, 220, 220, 220, 220, 220, 220, 225, 232
};

void setup() {
  mySerial.begin(9600);
  delay(100);
  mySerial.write(12); // clear
  delay(10);
  mySerial.write(22); // no cursor no blink
  delay(10);
  mySerial.write(17); // backlight
  delay(10);
  mySerial.print("Ice, Ice");
  mySerial.write(13);
  mySerial.print("Baby");
  for(long k=0; k<num; k++){
    
    mySerial.write(durs[k]); mySerial.write(octs[k]); mySerial.write(notes[k]);
    int len = 214 - durs[k];
    float del = 2000 / pow(2, len);
    delay(int(del*1.1));
  }

}

void loop() {

}
