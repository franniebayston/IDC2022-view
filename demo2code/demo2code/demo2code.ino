//
// 0. IMPORT LIBRARIES
//

// Servos
#include <Servo.h>
Servo servoLeft;
Servo servoRight;

// LCD
#include <SoftwareSerial.h>
#define TxPin 14
SoftwareSerial mySerial = SoftwareSerial(255, TxPin);

// IR Sensor
#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h>
IRTherm therm;

// Pins for QTI connections on board
#define lineSensor1 49
#define lineSensor2 51
#define lineSensor3 53

// Backup (Ultrasound)
const int pingPin = 35;

// Backup ???
# define Tx 16
# define num 17

// RGB
#define r 45
#define g 46
#define b 44

// Determine Stop Hash
int currentHash = -1;
int totalHash = -1;

// Store mission5 (cold) Hash
int missionHash = -1;
int doPrimary = -1;

// Reading boolean
int Reading = 0;
int nonZero = 0;

// Arrays
int group_scores[] = {0, 0, 0, 0, 0};
int sumScore = 0;
int modScore = 0;
int amtCounter = 0;

// SPEAKER SOUND
#define num 34
int durs[num]  = {211, 211, 211, 210, 210, 211, 211, 211, 211, 211, 211, 211, 210, 210, 211, 211, 212, 211, 211, 211, 210, 210, 211, 211, 211, 211, 211, 211, 211, 210, 210, 211, 211, 212};
int octs[num]  = {215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215};
int notes[num] = {220, 220, 220, 220, 220, 220, 225, 232, 220, 220, 220, 220, 220, 220, 220, 225, 232, 220, 220, 220, 220, 220, 220, 225, 232, 220, 220, 220, 220, 220, 220, 220, 225, 232};


//
// 1. FUNCTION DECLARATION
//
void veerLeft(int);
void veerRight(int);
void moveForward(int);
void halt(int);
void qti_setup();

void check_hash(int, int);
long rcTime(int pin);
void check_stop(int, int);

void set_RGB(int, int, int);
void hash_RGB(int, int);

void mission5();

void backupmission();
float microsecondsToInches(long);
float microsecondsToCentimeters(long);
float distance_in_inches();

int getGroupScore(int);
int getGroupNum(int);

void showScore(int, int);
void showSum(int);
void showMod(int);

void lightInd();
void lightInd2();

void modCountsZero();
void modCountsOne();
void modCountsTwo();
void play_song();


//
// 2. INITIALIZE SENSORS and SERIAL
//
void setup() {
//  Serial.begin(9600); // Start serial monitor to view output on computer
  Serial1.begin(9600);

  mySerial.begin(9600); // LCD setup
  delay(100);
  mySerial.write(12); // clears LCD
  delay(10);
  mySerial.write(22);
  delay(10);
  mySerial.write(17);
  delay(10);


  Serial2.begin(9600); // initialize XBee
  delay(500);

  Serial.println("Running diagnostics....\n");
  Serial.println("Serial monitor set up.");

  // Servos
  servoLeft.attach(11);
  servoRight.attach(12);
  Serial.println("Servos set up.");

  // RGB
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  analogWrite(r, 255);
  analogWrite(b, 255);
  analogWrite(g, 255);
  Serial.println("RGB on-board set up.");

  // PRIMARY OR BACKUP?? Boolean
  // True or False Wire at pin 5
  //
  doPrimary = digitalRead(5) ? 1 : 0; // Sets doPrimary to true if pin 5 is wired.
  Serial.println("doPrimary boolean set.");
  Serial.println(doPrimary);


  // PRIMARY MISSION 5
  // 5 or 6 Hashmark Wire
  //
  if (doPrimary)
  {
    pinMode(6, INPUT); // Uses pin 6 as the "switch"
    totalHash = digitalRead(6) ? 6 : 5;  // Sets the total Hashes to 6 if wired.
    Serial.println("Hashmark total set up.");
    Serial.println(totalHash);

    // Joining I2C bus: for Thermo
    Wire.begin();
    Serial.println("IR Part 1 of 2 set up.");

    // Initialize thermal IR
    therm.begin();
    Serial.println("IR Part 2/2 set up.");
    therm.setUnit(TEMP_F); // Set the library's units to Farenheit (sic)
  }
  else {
     // BACKUP MISSION
     // Ultrasound
     //
    totalHash = 6;

    pinMode(37, OUTPUT);
    pinMode(39, OUTPUT);
    digitalWrite(37, HIGH);
    digitalWrite(39, LOW);
  }

  pinMode(LED_BUILTIN, OUTPUT); // LED pin as output
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);

  // Final sanity check
  mySerial.write(12);
  delay(1000);

}

void loop() {
  //
  // QTI SENSOR READS. State calculated. ~ for line following
  //
  int qti1 = rcTime(lineSensor1);
  int qti2 = rcTime(lineSensor2);
  int qti3 = rcTime(lineSensor3);

  int threshold = 200; // de facto

  int sLeft = (qti1 < threshold) ? 1 : 0;
  int sMid = (qti2 < threshold) ? 1 : 0;
  int sRight = (qti3 < threshold) ? 1 : 0;

  int state = (4 * sLeft) + (2 * sMid) + (1 * sRight);
  //  Serial.println(state);
  //  Serial.println();

  //
  // Backup Mission Initialization
  //

  float backup_array[6];


  //
  // FUNCTION CALLS by STATE
  //
  switch (state)
  {
    case 0: // ON HASHMARK
      currentHash++;
      halt(250);
      
      while ((currentHash > (totalHash - 1)) && Reading) {
        showScore(5, missionHash + 1); // Print our mission score
        delay(100);
        
        if ((amtCounter % 10 == 0) && (amtCounter < 81)) {
          lightInd(); // color when sending our value
          delay(50);
          if (Serial2.available()) {
            Serial2.print((char) (missionHash + 1 + 84));
          }
        }
        else if (amtCounter >= 81) {
            
            sumScore = (group_scores[0] + group_scores[1] + group_scores[2] + group_scores[3] + missionHash + 1);
            modScore = sumScore%3;

            showSum(sumScore);
            delay(300);
            showMod(modScore);
            delay(300);
            
            if (modScore == 0) {
              modCountsZero();
            }

            else if (modScore == 1) {
              modCountsOne();
            }

            else {
              modCountsTwo();
            }

            while (1) {
             
              delay(100);
            }
          }
        else {
         
          if (Serial2.available()) {
            int rawChar = Serial2.read();
            delay(100);
            lightInd2();
            delay(100);
          
            int temp_group = getGroupNum(rawChar);
            int temp_score = getGroupScore(rawChar);
            showScore(temp_group, temp_score);

            // update to array.
            group_scores[temp_group] = temp_score;

           
            
          }
          // check if sum can be calculated (assumes break works)

        
         
         } // after else
         
        amtCounter +=1;
      
      }

      //
      // MISSION
      //
      if (doPrimary) {
        if (currentHash <= (totalHash - 1)) {
          mission5();
        }
        // Stop moving at Last Hash
        check_stop(totalHash - 1, currentHash);
        // Keep moving if not Last Hash
        moveForward(200);
      }

      else { // BACKUP
        // flash LED to show we doing stuff
        analogWrite(r, 255);
        analogWrite(g, 0);
        analogWrite(b, 255);
        delay(500);
        analogWrite(r, 255);
        analogWrite(g, 255);
        analogWrite(b, 255);
        delay(1000);

        // for the first five hashes {0, 1, 2, 3, 4}
        if (currentHash <= (totalHash - 2)) {
          backup_array[currentHash] = distance_in_inches();
          moveForward(200);
        }

        // back to the start {5}
        else if (currentHash == (totalHash - 1)) {
          check_stop(1, 1); // Always True
          float smallest = backup_array[0];

          missionHash = 1;
          for (int i = 0; i <= 5; i++) {
            if (backup_array[i] < smallest) {
              smallest = backup_array[i];
              missionHash = i + 1;
            }
          }

          lightInd();
          delay(100);
          
          Serial2.print((char) (missionHash + 1 + 84));
          
          delay(500);
          
          group_scores[4] = (missionHash + 1);
          
        }
        
        else {
          //          Serial.print("Backup done. missionHash found.");
        }
      }


      break;


    case 1: // Coming on a left angle
      veerLeft(1);
      break;

    case 2: // Highly unlikely
      break;


    case 3:
      veerLeft(1);
      break;


    case 4:
      veerRight(1);
      break;


    case 5:
      moveForward(1);
      break;


    case 6:
      veerRight(1);
      break;


    case 7:
      Serial.println("Not on line");
      break;


    default:
      halt(100);
      Serial.println("Uh oh. We not moving.");
      break;


  }
}



long rcTime(int pin) {
  pinMode(pin, OUTPUT);    // Sets pin as OUTPUT
  digitalWrite(pin, HIGH); // Pin HIGH
  delay(1);                // Waits for 1 millisecond
  pinMode(pin, INPUT);     // Sets pin as INPUT
  digitalWrite(pin, LOW);  // Pin LOW
  long time = micros();    // Tracks starting time
  while (digitalRead(pin)); // Loops while voltage is high
  time = micros() - time;  // Calculate decay time
  return time;             // Return decay time
}

void veerLeft(int sec) {
  servoLeft.writeMicroseconds(1500 + 25);
  servoRight.writeMicroseconds(1500 - 60);
  delay(sec);
}

void veerRight(int sec) {
  // previous
  //  servoLeft.writeMicroseconds(1500 + 40);
  //  servoRight.writeMicroseconds(1500 + 25);
  servoLeft.writeMicroseconds(1500 + 25);
  servoRight.writeMicroseconds(1500 - 10);
  delay(sec);
}

void moveForward(int sec) {
  // previous
  // servoLeft.writeMicroseconds(1500 + 200);
  // servoRight.writeMicroseconds(1500 - 200);
  servoLeft.writeMicroseconds(1500 + 50);
  servoRight.writeMicroseconds(1500 - 50);
  delay(sec);
}

void halt(int sec) {
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1500);
  delay(sec);
}

void tshootLeft() {
  servoLeft.writeMicroseconds(1700);
  servoRight.writeMicroseconds(1500);
}

void tshootRight() {
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1700);
}

void check_stop(int totalHashes, int currentHash) {
  if (currentHash == totalHashes) {
    servoLeft.detach();
    servoRight.detach();
  }

  // set true to time to start Reading
  Reading = 1;
}

void set_RGB(int red, int green, int blue) {
  analogWrite(r, 255 - red);
  analogWrite(b, 255 - blue);
  analogWrite(g, 255 - green);
}

void hash_RGB(int totalHashes, int currentHash) {
  if (totalHashes == 1) {
    switch (currentHash) {
      case 0:
        set_RGB(255, 0, 0); //red
        break;
      case 1:
        set_RGB(255, 255, 0); //yellow
        break;
      case 2:
        set_RGB(0, 255, 0); //green
        break;
      case 3:
        set_RGB(0, 0, 255); //blue
        break;
      case 4:
        set_RGB(0, 100, 100); //cyan
        break;
      case 5:
        set_RGB(102, 0, 204); // magenta
        break;
      default:
        set_RGB(0, 0, 0);
        break;

    }

  }
  else {
    switch (currentHash) {
      case 0:
        set_RGB(255, 0, 0); //red
        break;
      case 1:
        set_RGB(255, 255, 0); //yellow
        break;
      case 2:
        set_RGB(0, 255, 0); //green
        break;
      case 3:
        set_RGB(0, 0, 255); //blue
        break;
      case 4:
        set_RGB(0, 100, 100); //cyan
        break;
      default:
        set_RGB(0, 0, 0);
        break;
    }
  }
}

void mission5() {

  digitalWrite(LED_BUILTIN, HIGH); // turn on light to show sensor doing stuff
  delay(500);

  // IR Sensor READS TEMPERATURE
  if (therm.read())
  {
    if (therm.object() < (therm.ambient() - 25))
    {

      set_RGB(0, 0, 255);
      delay(500);

      missionHash = currentHash;

      set_RGB(0, 0, 0);
      delay(1000);

      set_RGB(255, 255, 255); // turn on to show XBee communication

      // Communicate to XBee
      
      Serial2.print((char) (missionHash + 1 + 84));
      
      delay(100);
      group_scores[4] = (missionHash + 1);

      delay(500);
      set_RGB(0, 0, 0);
    }
    else {

      set_RGB(255, 0, 0);
      delay(500);
      set_RGB(0, 0, 0);
    }

  }

  digitalWrite(LED_BUILTIN, LOW);

  //
  //  Serial.print("Object: " + String(therm.object(), 2));
  //  Serial.println("F");
  //  Serial.print("Ambient: " + String(therm.ambient(), 2));
  //  Serial.println("F");
  //  Serial.println();
}

float microsecondsToInches(long microseconds) {
  // The speed of sound is about 1125 ft/s
  // Sound takes about 74.074 us to travel 1 in
  return microseconds / 74.074 / 2;
}

float microsecondsToCentimeters(long microseconds) {
  // The speed of sound is about 343 m/s
  // Sound takes about 29.155 us to travel 1 cm
  return microseconds / 29.155 / 2.0;
}

float distance_in_inches() {
  long duration;
  float inches, cm;
  // short LOW pulse to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  // 2 ms HIGH pulse
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  // back to LOW
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from thPINGe ))):
  // a HIGH pulse whose duration is the time (in microseconds)
  // from the sending of the ping to the reception of
  // its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);

  //  Serial.print(inches);
  //  Serial.print("in, ");
  //  Serial.print(cm);
  //  Serial.print("cm");
  //  Serial.println();

  return inches;
}

int getGroupNum(int rawChar) {
  int temp = ((((rawChar - 65) - ((rawChar - 65) % 5)) / 5) + 1);
  return temp;
}

int getGroupScore(int rawChar) {
  int temp = ((rawChar % 5) + 1);
  return temp;
}

void showScore(int group, int score) {

  switch (group) {

    case 1:

      mySerial.write(128);

      mySerial.print("1=");

      mySerial.print(score);

      break;



    case 2:

      mySerial.write(132);

      mySerial.print("2=");

      mySerial.print(score);

      break;



    case 3:

      mySerial.write(136);

      mySerial.print("3=");

      mySerial.print(score);

      break;



    case 4:

      mySerial.write(140);

      mySerial.print("4=");

      mySerial.print(score);

      break;



    case 5:

      mySerial.write(148);

      mySerial.print("5=");

      mySerial.print(score);

      break;
  }
}


void showMod(int mod) {
  mySerial.write(157);
  mySerial.print("M=");
  mySerial.print(mod);
}

void showSum(int sum) {
  mySerial.write(152);
  mySerial.print("S=");
  mySerial.print(sum);
}

void lightInd() {

  // flash onboard
  analogWrite(r, 100);
  analogWrite(g, 150);
  analogWrite(b, 100);
  delay(500);
  analogWrite(r, 255);
  analogWrite(g, 255);
  analogWrite(b, 255);
  delay(500);

}

void lightInd2() {
  // flash onboard
  analogWrite(r, 255);
  analogWrite(g, 255);
  analogWrite(b, 0);
  delay(500);
  analogWrite(r, 255);
  analogWrite(g, 255);
  analogWrite(b, 255);
  delay(500);
}

//MODCOUNTSZERO: SONG 
void modCountsZero(){
  play_song();
}

//MODCOUNTSONE: LIGHT SHOW
void modCountsOne(){
  for(int i = 0; i < 100; i++){
      int x = i % 3;

      switch(x){
        case 0:
          analogWrite(r, 0);
          analogWrite(g, 255);
          analogWrite(b, 255);
          delay(100);
          break;
        case 1:
          analogWrite(r, 255);
          analogWrite(g, 0);
          analogWrite(b, 255);
          delay(100);
          break;
        case 2:
          analogWrite(r, 255);
          analogWrite(g, 255);
          analogWrite(b, 0);
          delay(100);
          break;
      }
     
    }
}

//MODCOUNTSTWO: Dance
void modCountsTwo(){
  bot_dance();
}

//SONG FOR PIEZOSPEAKER 
void play_song() {
   for(long k=0; k<num; k++){
    mySerial.write(durs[k]); mySerial.write(octs[k]); mySerial.write(notes[k]);
    int len = 214 - durs[k];
    float del = 2000 / pow(2, len);
    delay(int(del*1.1));
  }
}
  
//SERVO CODE FOR DANCE
void bot_dance() {
  servoLeft.attach(11);
  servoRight.attach(12);

  danceBackward();
  delay(1500);
//  danceForward();
//  delay(500);
  
  // 360 SPINNNNNNN
  servoLeft.writeMicroseconds(1700);
  servoRight.writeMicroseconds(1600);
  delay(3000);
  
  Serial.println("SHIMMY");
  danceForwardShimmy();
  danceBackwardShimmy();
  danceForwardShimmy();
  danceBackwardShimmy();
  danceForwardShimmy();
  danceBackwardShimmy();
  danceForwardShimmy();
  danceBackwardShimmy();
  danceForwardShimmy();
  // 90 SPINNNNNNN
  servoLeft.writeMicroseconds(1700);
  servoRight.writeMicroseconds(1600);
  delay(1000);
  danceBackwardShimmy();
  danceForwardShimmy();
  danceBackwardShimmy();
  danceForwardShimmy();
  danceBackwardShimmy();
  danceForwardShimmy();
  danceBackwardShimmy();
  danceForwardShimmy();
  danceBackwardShimmy();
  
  servoLeft.detach();
  servoRight.detach();
}
void danceBackward() {
  servoLeft.writeMicroseconds(1500 - 50);
  servoRight.writeMicroseconds(1500+50);

  }
void danceForwardShimmy() {
  danceQuickLeft();
  delay(300);
  danceQuickRight();
  delay(300);
}
void danceQuickLeft() {
  servoLeft.writeMicroseconds(1500 );
  servoRight.writeMicroseconds(1500 - 250);
}
void danceQuickRight() {
  servoLeft.writeMicroseconds(1500 + 250);
  servoRight.writeMicroseconds(1500);
}
void danceBackwardShimmy() {
  danceBQLeft();
  delay(300);
  danceBQRight();
  delay(300);
}
void danceBQLeft() {
  servoLeft.writeMicroseconds(1500 - 250);
  servoRight.writeMicroseconds(1500);
}
void danceBQRight() {
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1500 + 250);
}
