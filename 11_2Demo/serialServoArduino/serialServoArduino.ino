//code to get motor control started with OpenCV

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

const int servoPin1 = 9; // the pin that the servo is attached to
int incomingByte;      // a variable to read incoming serial data into

int pos = 90;    // variable to store the servo position


void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  // initialize the LED pins as an output:
  pinMode(servoPin1, OUTPUT);

  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(pos);  //tells servo to go to the middle (90 degrees) first
  delay(1000); //waits a second for the motor to initialize position
}

void loop() {
  // see if there's incoming serial data:
  if (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    incomingByte = Serial.read();
    // if it's an L, turn toggle LED1:
    if (incomingByte == 'L') {
      pos = pos + 1;
    }
    // if it's an R, toggle LED2:
    if (incomingByte == 'R') {
      pos = pos - 1;
    }
    myservo.write(pos);
    delay(15);
  }
}
