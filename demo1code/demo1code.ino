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

// RGB 
#define r 45
#define g 46
#define b 44

// Determine Stop Hash
int currentHash = -1;
int totalHash = -1;

// Store missionHash
int missionHash = -1;

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

  // Initialize Serials (XBee and Arduino Serial)
  Serial.begin(9600); //start the serial monitor so we can view the output
  Serial2.begin(9600); // Xbee Serial
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

  // PRIMARY MISSION 5
  // 5 or 6 Hashmark Wire
  //
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

  pinMode(LED_BUILTIN, OUTPUT); // LED pin as output
  digitalWrite(LED_BUILTIN, HIGH);
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
  // FUNCTION CALLS by STATE
  //
  switch (state)
  {
    case 0: // ON HASHMARK
      currentHash++;
      Serial.println("Current Hash:");
      Serial.println(currentHash);

      
      halt(250);

      //
      // MISSION 5
      //
      if (currentHash <= (totalHash-1)) {
          mission5();
      }
      // Stop moving at Last Hash
      check_stop(totalHash-1, currentHash);
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
      
      set_RGB(0,0,0);
      delay(1000);
      
      set_RGB(255, 255, 255); // turn on to show XBee communication

      // Communicate to XBee
      Serial2.print(missionHash+1);
       
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
