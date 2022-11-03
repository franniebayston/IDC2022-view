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

// Arrays
int group_scores[] = { -1, -1, -1, -1, -1};
int printed[] = {0, 0, 0, 0, 1};
int groupSum = 0;

int getSum = 0;

int amtCounter = 0;



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


//
// 2. INITIALIZE SENSORS and SERIAL
//
void setup() {
//  Serial.begin(9600);
  Serial1.begin(9600);
  // Initialize Serials (XBee and LCD)
//  pinMode(TxPin, OUTPUT);
//  digitalWrite(TxPin, HIGH);
  mySerial.begin(9600); //start the serial monitor so we can view the output
  delay(100);
  mySerial.write(12); // clears LCD
  delay(10);
  mySerial.write(22);
  delay(10);
  mySerial.write(17);
  delay(10);
//  
//  digitalWrite(TxPin, LOW);
//  delay(100);

  Serial2.begin(9600);
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
  doPrimary = digitalRead(5) ? 1 : 0; // Sets doPriamry to true if pin 5 is wired.
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

  // BACKUP MISSION
  // Ultrasound
  //
  if (!doPrimary) {
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
      //      Serial.println("Current Hash:");
      //      Serial.println(currentHash);
      halt(250);

      // continuous loop that just focuses on reading the incoming data
      while ((currentHash > (totalHash - 1)) && (Reading)) {
        showScore(5, missionHash + 1);

        lightInd();
        if (amtCounter % 100 == 0) {
          if (Serial2.available()) {
            Serial2.print((char) (missionHash + 1 + 84));
          }
          delay(500);
        }
        else {

          if (Serial2.available()) {
          int rawChar = Serial2.read();
          set_RGB(255, 0, 255);
          delay(500);
          Serial.println(rawChar);
          
          // expect a meaningful char to be 65 ASCII or greater
          
//          lightInd();
//          delay(100);
          int temp_group = getGroupNum(rawChar);
          int temp_score = getGroupScore(rawChar);
        
          showScore(temp_group, temp_score);

          set_RGB(0,0,0);
//
//          // update to array.
//          group_scores[temp_group] = temp_score;
//
//          if ((printed[temp_group] == 0) && (temp_score != -1)) {
//            showScore(temp_group, temp_score);
//            printed[temp_group] = 1;
//          }
//          
//        }
//        // check if sum can be calculated (assumes break works)
//
//        for (int i = 0; i <= 4; i++) {
//          if (group_scores[i] == -1) {
//            getSum = 0;
//            groupSum = 0;
//            break;
//          }
//          else {
//            groupSum += group_scores[i];
//            getSum = 1;
//          }
//        }
//
//        if (getSum) {
//          Reading = 0;
//          showSum(groupSum);
//          delay(100);
//        }



      }
      }
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

        if (currentHash <= (totalHash - 2)) {
          backup_array[currentHash] = distance_in_inches();
          moveForward(200);
        }
        else if (currentHash == (totalHash - 1)) {
          check_stop(1, 1); // Always True

          float smallest = backup_array[0];
          int missionHash = 0;

          for (int i = 0; i < 6; i++) {
            if (backup_array[i] < smallest) {
              smallest = backup_array[i];
              missionHash = i;
            }
          }

          lightInd();
          
          Serial2.print((char) (missionHash + 1 + 84));
          
          delay(500);
          
          group_scores[4] = (missionHash + 1 + 84);
     

          //          Serial.println("Found missionHash.");
          //          Serial.println(missionHash + 1);
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
      group_scores[4] = (missionHash + 1 + 84);

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
  analogWrite(r, 255);
  analogWrite(g, 0);
  analogWrite(b, 255);
  delay(500);
  analogWrite(r, 255);
  analogWrite(g, 255);
  analogWrite(b, 255);
  delay(500);

}
