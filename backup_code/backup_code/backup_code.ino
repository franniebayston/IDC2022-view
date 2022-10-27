const int pingPin = 35;

#define Rx 17
#define Tx 16

int hashNum = 0;

void setup() {
  Serial.begin(9600);   //serial monitor
  Serial2.begin(9600);  //XBee
  pinMode(37, OUTPUT);
  pinMode(39, OUTPUT);
  digitalWrite(37, HIGH);
  digitalWrite(39, LOW);
  servoLeft.attach(12);
  servoRight.attach(11);

  
}

void loop() {
  
  int leftReading = rcTime(leftSensor); 
  int middleReading = rcTime(middleSensor); 
  int rightReading = rcTime(rightSensor); 
  
  int state = 4 * (leftReading < 275) + 2 * (middleReading < 275) + (rightReading<275);

  float array[6];
  
  switch(state) 
  {    
    case 0:
      stopnow();
      hashNum++;
      Serial.println(hashNum);

      //flash onboard LED to demonstrate
      analogWrite(redpin, 255);
      analogWrite(greenpin, 0);
      analogWrite(bluepin, 255);
      delay(500);
      analogWrite(redpin, 255);
      analogWrite(greenpin, 255);
      analogWrite(bluepin, 255);
      delay(1500); 

      if(hashNum ==1){
        array[1] = distance_in_inches();
      }
      if(hashNum ==2){
        array[2] = distance_in_inches();
      }
      if(hashNum ==3){
        array[3] = distance_in_inches();
      }
      if(hashNum ==4){
        array[4] = distance_in_inches();
      }
      if(hashNum ==5){
        array[5] = distance_in_inches();
      }
      if(hashNum ==6){
         stopfinal();
        float smallest = array[1];
        int index_of_smallest = 1;

        for (int i = 1; i <= 5; i++) {
            if (array[i] < smallest) {
                smallest = array[i];
                index_of_smallest = i;
            }
        }
        Serial2.print(index_of_smallest);
      
      }

      straight();
      delay(350);
      break;
  
    case 1:
    case 3:
      left();
      delay(50);
      break; 

    case 4:
    case 6:
      right();
      delay(50);
      break; 
 
    case 5:
      straight();
      delay(50);
      break;

    case 7:
      straight();
      delay(50);
      break; 
  }
}
void left(){
   servoLeft.writeMicroseconds(1480);
   servoRight.writeMicroseconds(1480);
}
void straight(){
   servoLeft.writeMicroseconds(1550);
   servoRight.writeMicroseconds(1455);
  
}
void right(){
   servoLeft.writeMicroseconds(1520);
   servoRight.writeMicroseconds(1520);
}
void stopfinal(){
   servoLeft.detach();
   servoRight.detach();
}

void stopnow() {
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1500);
}


long rcTime(int pin){
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

void lightOff(){
    delay(10);
    analogWrite(redpin, 255);
    analogWrite(greenpin, 255);
    analogWrite(bluepin, 255);
}


float distance_in_inches(){
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
  
      Serial.print(inches);
      Serial.print("in, ");
      Serial.print(cm);
      Serial.print("cm");
      Serial.println();

      return inches;
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
