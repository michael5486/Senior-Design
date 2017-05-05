int leftA1 = 3;// int leftForwardLEDPin = 10;       // LED connected to digital pin 9
int leftA2 = 4;//int leftBackwardLEDPin = 9;    // LED connected to digital pin 10
int rightA1 = 5;//int rightForwardLEDPin = 6;     // LED connected to digital pin 5
int rightA2 = 6;//int rightBackwardLEDPin = 5;    // LED connected to digital pin 6


int incomingByte = 0;

int leftCommand = 0;
int rightCommand = 0;

void parseByte(byte);
void controlMotors();

void setup() {
  Serial.begin(9600);
}

void loop() {
//Output to LEDs when data is received
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    parseByte(incomingByte);
    controlMotors();
  }

}

//takes a byte of input, parses it, and sets leftLEDPin and rightLEDpint accordingly
void parseByte(byte val) {  
  //binary mask 11110000
  int leftMask = 0b11110000;
  //binary mask 00001111
  int rightMask = 0b00001111;

  //binary AND to get bits for leftMotor
  int tempLeft = (val & leftMask) >> 4;
  //binary AND to get bits for rightMotor;
  int tempRight = val & rightMask;

  leftCommand = tempLeft;
  rightCommand = tempRight;
}

//parses leftLEDCommand and rightLEDCommand to send forward/reverse to each LED
//two LEDs for each motor, for forward and reverse
void controlMotors() {

    //clears input to each pin
    analogWrite(leftA1, 255);
    analogWrite(leftA2, 255);
    analogWrite(rightA1, 255);
    analogWrite(rightA2, 255); 
    

  switch(leftCommand) {
    case 0: // Left Wheel Backward Full -- Needs to go CCW
      analogWrite(leftA1, 0);
      analogWrite(leftA2, 255);
    break;
    case 1: 
      analogWrite(leftA1, 30);
      analogWrite(leftA2, 255);
    break;
    case 2:
      analogWrite(leftA1, 60);
      analogWrite(leftA2, 255);
    break;
    case 3:
      analogWrite(leftA1, 90);
      analogWrite(leftA2, 255);
    break;
    case 4:
      analogWrite(leftA1, 120);
      analogWrite(leftA2, 255);
    break;
    case 5:
      analogWrite(leftA1, 150);
      analogWrite(leftA2, 255);
    break;
    case 6: // Left Backward Low
      analogWrite(leftA1, 180);
      analogWrite(leftA2, 255);
    break;
    //case 7: NOTHING
      //analogWrite(leftBackwardLEDPin, 0);
    //break;
    case 8: // Left Wheel Forward Low -- Needs to go CW
      analogWrite(leftA1, 180);
      analogWrite(leftA2, 180);
    break;
    case 9:
      analogWrite(leftA1, 150);
      analogWrite(leftA2, 150);
    break;
    case 10:
      analogWrite(leftA1, 120);
      analogWrite(leftA2, 120);
    break;
    case 11:
      analogWrite(leftA1, 90);
      analogWrite(leftA2, 90);
    break;
    case 12:
      analogWrite(leftA1, 60);
      analogWrite(leftA2, 60);
    break;
    case 13:
      analogWrite(leftA1, 30);
      analogWrite(leftA2, 30);
    break;
    case 14: // Left Forward Full
      analogWrite(leftA1, 0);
      analogWrite(leftA2, 0);
    break;
  }

    switch(rightCommand) { // Right Wheel Set
    case 0: // Right Wheel Backward Full -- Needs to go CW
      analogWrite(rightA1, 0);
      analogWrite(rightA2, 0);
    break;
    case 1:
      analogWrite(rightA1, 30);
      analogWrite(rightA2, 30);
    break;
    case 2:
      analogWrite(rightA1, 60);
      analogWrite(rightA2, 60);
    break;
    case 3:
      analogWrite(rightA1, 90);
      analogWrite(rightA2, 90);
    break;
    case 4:
      analogWrite(rightA1, 120);
      analogWrite(rightA2, 120);
    break;
    case 5:
      analogWrite(rightA1, 150);
      analogWrite(rightA2, 150);
    break;
    case 6: // Right Backward 0
      analogWrite(rightA1, 180);
      analogWrite(rightA2, 180);
    break;
    case 7: 
      //analogWrite(rightBackwardLEDPin, 0);
      analogWrite(rightA1, 255);
      analogWrite(rightA2, 255);
    break;
    case 8: // Right Wheel Forward 0 -- Needs to go CCW
      analogWrite(rightA1, 180);
      analogWrite(rightA2, 255);
    break;
    case 9:
      analogWrite(rightA1, 150);
      analogWrite(rightA2, 255);
    break;
    case 10:
      analogWrite(rightA1, 120);
      analogWrite(rightA2, 255);
    break;
    case 11:
      analogWrite(rightA1, 90);
      analogWrite(rightA2, 255);
    break;
    case 12:
      analogWrite(rightA1, 60);
      analogWrite(rightA2, 255);
    case 13:
      analogWrite(rightA1, 30);
      analogWrite(rightA2, 255);
    case 14: // Right Forward Full
      analogWrite(rightA1, 0);
      analogWrite(rightA2, 255);
    break;
  }
}  




