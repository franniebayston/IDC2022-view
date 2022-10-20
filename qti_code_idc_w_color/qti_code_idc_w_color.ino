#include <Servo.h>
//Pins for QTI connections on board
#define lineSensor1 49
#define lineSensor2 51
#define lineSensor3 53

#define r 45
#define g 46
#define b 44

Servo servoLeft;
Servo servoRight;
int currentHash = -1;
int numhash = 0;

void veerLeft(int);
void veerRight(int);
void moveForward(int);
void halt(int);
void check_hash(int, int);

void set_RGB(int, int, int);
void hash_RGB(int, int);

void setup() {
  Serial.begin(9600); //start the serial monitor so we can view the output
  
  // Servos
  servoLeft.attach(11);
  servoRight.attach(12);

  // RGB
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  analogWrite(r, 255);
  analogWrite(b, 255);
  analogWrite(g, 255);
  
  // input 5 or six hashmark
  pinMode(5, INPUT);
  numhash = digitalRead(5);
}

void loop() {
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
  int state = (4*sLeft) + (2*sMid) + (1*sRight);
  Serial.println(state);
  Serial.println();
  

  switch (state) {
    case 0:
      currentHash += 1;
      Serial.println(currentHash);
      halt(250);
      hash_RGB(numhash, currentHash);
      delay(200);
      set_RGB(0, 0, 0);
      check_stop(numhash, currentHash);
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
  while(digitalRead(pin)); // Loops while voltage is high
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

void set_RGB(int red, int green, int blue) {
  analogWrite(r, 255-red);
  analogWrite(b, 255-blue);
  analogWrite(g, 255-green);
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
