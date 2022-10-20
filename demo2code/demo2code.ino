//
// 0. IMPORT LIBRARIES
//

// import Servo
#include <Servo.h>

//LCD
#include <SoftwareSerial.h>

// IR Sensor
#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> 

//Pins for QTI connections on board
#define lineSensor1 49
#define lineSensor2 51
#define lineSensor3 53

// Create an IRTherm object to interact with throughout
IRTherm therm; 

// LCD
#define TxPin 14
SoftwareSerial mySerial = SoftwareSerial(255, TxPin);


#define num 17

// Servos
Servo servoLeft;
Servo servoRight;

// RGB 
#define r 45
#define g 46
#define b 44


// Determine Stop Hash
int currentHash = -1;
int numhash = -1;

// Store mission5 (cold) Hash
int mission5Hash = -1;

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

void mission5(int, int);


//
// 2. INITIALIZE SENSORS and SERIAL
//
void setup() {
  
  Serial.begin(9600); //start the serial monitor so we can view the output
  Serial2.begin(9600); // Xbee Serial
  delay(500);
  Serial.println("Running diagnostics....\n");
  Serial.println("Serial monitor set up.");
  
  Wire.begin(); //Joining I2C bus
  Serial.println("IR Part 1 of 2 set up.");

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

  // 5 or 6 Hashmark Switch
  pinMode(5, INPUT);
  numhash = digitalRead(5) ? 6 : 5;
  Serial.println("Hashmark total set up.");
  Serial.println(numhash);


// initialize thermal IR
  therm.begin();
  Serial.println("IR Part 2/2 set up.");
  therm.setUnit(TEMP_F); // Set the library's units to Farenheit (sic)

  
  pinMode(LED_BUILTIN, OUTPUT); // LED pin as output

//  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);

  }

void loop() {

  //
  // QTI SENSOR READS. State calculated. ~ for line following
  //
  int qti1 = rcTime(lineSensor1);
  int qti2 = rcTime(lineSensor2);
  int qti3 = rcTime(lineSensor3);

  int threshold = 200; //TBD

  int sLeft = (qti1 < threshold) ? 1 : 0;
  int sMid = (qti2 < threshold) ? 1 : 0;
  int sRight = (qti3 < threshold) ? 1 : 0;

  int state = (4 * sLeft) + (2 * sMid) + (1 * sRight);
  Serial.println(state);
  Serial.println();

  //
  // FUNCTION CALLS by STATE
  //
  switch (state) {
    case 0: // ON HASHMARK
      currentHash += 1;
      Serial.println(currentHash);
      halt(250);

      // MISSION 5
      if (currentHash >= numhash) {
        mission5();
      }

      // Stop moving at Last Hash
      check_stop(numhash, currentHash);

      // Keep moving if not Last Hash
      moveForward(200);
      break;
      
    case 1: // Coming on a left angle
      veerLeft(1);
      break;
    case 2: // Highly unlikely
      Serial.println("What the (switch)???");
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


long rcTime(int pin)
{
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
  servoLeft.writeMicroseconds(1500 + 40);
  servoRight.writeMicroseconds(1500 + 25);
  delay(sec);
}

void moveForward(int sec) {
  servoLeft.writeMicroseconds(1500 + 200);
  servoRight.writeMicroseconds(1500 - 200);
  delay(sec);
}

void halt(int sec) {
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1500);
  delay(sec);
}

void doTest() {
  servoLeft.writeMicroseconds(1700);
  servoRight.writeMicroseconds(1500);
}

void check_stop(int totalHashes, int currentHash) {
  if (totalHashes == 1) {
    if (currentHash == 5) {
      servoLeft.detach();
      servoRight.detach();
      return;
    }
    return;
  }
  else {
    if (currentHash == 4) {
      servoLeft.detach();
      servoRight.detach();
      return;
    }
    return;
  }
}

void set_RGB(int red, int green, int blue) {
  analogWrite(r, 255-red);
  analogWrite(b, 255-blue);
  analogWrite(g, 255-green);
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

void qti_setup() {
  //  Serial.println(numhash);
  int qti1 = rcTime(lineSensor1);
  int qti2 = rcTime(lineSensor2);
  int qti3 = rcTime(lineSensor3);

  int threshold = 200; //TBD

  int sLeft = (qti1 < threshold) ? 1 : 0;
  int sMid = (qti2 < threshold) ? 1 : 0;
  int sRight = (qti3 < threshold) ? 1 : 0;

  //  Serial.println(qti1);
  //  Serial.println(qti2);
  //  Serial.println(qti3);
  //
  int state = (4 * sLeft) + (2 * sMid) + (1 * sRight);
//  Serial.println(state);
//  Serial.println();
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
      
      mission5Hash = currentHash;
      
      set_RGB(0,0,0);
      delay(1000);
      
      set_RGB(255, 255, 255); // turn on to show XBee communication

      // Communicate to XBee
      Serial2.print(mission5Hash+1);
       
      delay(500);
      set_RGB(0,0,0);
    }
    else {
      
        set_RGB(255, 0, 0);
        delay(500);
        set_RGB(0,0,0);
    }
    
  }

  digitalWrite(LED_BUILTIN, LOW);
   
  //
  Serial.print("Object: " + String(therm.object(), 2));
  Serial.println("F");
  Serial.print("Ambient: " + String(therm.ambient(), 2));
  Serial.println("F");
  Serial.println();
  }
  // QTI Sensor


  // receive
  //  if (Serial2.available()) {
  //    char incoming = Serial2.read();
  //    Serial.println(incoming);
  
