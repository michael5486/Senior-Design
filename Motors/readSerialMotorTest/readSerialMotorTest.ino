int leftA1 = 9;// int leftForwardLEDPin = 10;       // LED connected to digital pin 9
int leftA2 = 10;//int leftBackwardLEDPin = 9;    // LED connected to digital pin 10
int rightA1 = 5;//int rightForwardLEDPin = 6;     // LED connected to digital pin 5
int rightA2 = 6;//int rightBackwardLEDPin = 5;    // LED connected to digital pin 6


int incomingByte = 0;

int leftCommand = 0;
int rightCommand = 0;

void parseByte(int);
void controlLEDs();

void setup() {
  Serial.begin(9600);
}

void loop() {
//Output to LEDs when data is received
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    parseByte(incomingByte);
    controlLEDs();
  }

//  for (int i = 0; i <= 255; i++) {
//    parseByte(i);
//    Serial.println(i);
//    Serial.print("leftLEDCommand ");
//    Serial.print(leftLEDCommand);
//    Serial.print("rightLEDCommand");
//    Serial.print(rightLEDCommand);
//    Serial.print("\n");
//    
//    delay(250);
//    controlLEDs();
//    
//  }
}

//takes a byte of input, parses it, and sets leftLEDPin and rightLEDpint accordingly
void parseByte(int byte) {  
  //binary mask 11110000
  int leftMask = 0b11110000;
  //binary mask 00001111
  int rightMask = 0b00001111;

  //binary AND to get bits for leftMotor
  int tempLeft = (byte & leftMask) >> 4;
  //binary AND to get bits for rightMotor;
  int tempRight = byte & rightMask;

  leftCommand = tempLeft;
  rightCommand = tempRight;
}

//parses leftLEDCommand and rightLEDCommand to send forward/reverse to each LED
//two LEDs for each motor, for forward and reverse
void controlLEDs() {

    //clears input to each pin
    analogWrite(leftA1, 0);
    analogWrite(leftA2, 0);
    analogWrite(rightA1, 0);
    analogWrite(rightA2, 0);
    

  switch(leftCommand) {
    case 0: // Left Wheel Backward Full -- Needs to go CW
      analogWrite(leftA1, 255);
      analogWrite(leftA2, 255);
    break;
    case 1: 
      analogWrite(leftA1, 225);
      analogWrite(leftA2, 225);
    break;
    case 2:
      analogWrite(leftA1, 195);
      analogWrite(leftA2, 195);
    break;
    case 3:
      analogWrite(leftA1, 165);
      analogWrite(leftA2, 165);
    break;
    case 4:
      analogWrite(leftA1, 135);
      analogWrite(leftA2, 135);
    break;
    case 5:
      analogWrite(leftA1, 105);
      analogWrite(leftA2, 105);
    break;
    case 6: // Left Backward 0
      analogWrite(leftA1, 0);
      analogWrite(leftA2, 0);
    break;
    //case 7: NOTHING
      //analogWrite(leftBackwardLEDPin, 0);
    //break;
    case 8: // Left Wheel Forward 0 -- Needs to go CCW
      analogWrite(leftA1, 0);
      analogWrite(leftA2, 0);
    break;
    case 9:
      analogWrite(leftA1, 105);
      analogWrite(leftA2, 255);
    break;
    case 10:
      analogWrite(leftA1, 135);
      analogWrite(leftA2, 255);
    break;
    case 11:
      analogWrite(leftA1, 165);
      analogWrite(leftA2, 255);
    break;
    case 12:
      analogWrite(leftA1, 195);
      analogWrite(leftA2, 255);
    break;
    case 13:
      analogWrite(leftA1, 225);
      analogWrite(leftA2, 255);
    break;
    case 14: // Left Forward Full
      analogWrite(leftA1, 255);
      analogWrite(leftA2, 255);
    break;
  }

    switch(rightCommand) { // Right Wheel Set
    case 0: // Right Wheel Backward Full -- Needs to go CCW
      analogWrite(rightA1, 255);
      analogWrite(rightA2, 255);
    break;
    case 1:
      analogWrite(rightA1, 225);
      analogWrite(rightA2, 255);
    break;
    case 2:
      analogWrite(rightA1, 195);
      analogWrite(rightA2, 255);
    break;
    case 3:
      analogWrite(rightA1, 165);
      analogWrite(rightA2, 255);
    break;
    case 4:
      analogWrite(rightA1, 135);
      analogWrite(rightA2, 255);
    break;
    case 5:
      analogWrite(rightA1, 105);
      analogWrite(rightA2, 255);
    break;
    case 6: // Right Backward 0
      analogWrite(rightA1, 0);
      analogWrite(rightA2, 0);
    break;
    //case 7: NOTHING
      //analogWrite(rightBackwardLEDPin, 0);
    //break;
    case 8: // Right Wheel Forward 0 -- Needs to go CW
      analogWrite(rightA1, 0);
      analogWrite(rightA2, 0);
    break;
    case 9:
      analogWrite(rightA1, 105);
      analogWrite(rightA2, 105);
    break;
    case 10:
      analogWrite(rightA1, 135);
      analogWrite(rightA2, 135);
    break;
    case 11:
      analogWrite(rightA1, 165);
      analogWrite(rightA2, 165);
    break;
    case 12:
      analogWrite(rightA1, 195);
      analogWrite(rightA2, 195);
    case 13:
      analogWrite(rightA1, 225);
      analogWrite(rightA2, 225);
    case 14: // Right Forward Full
      analogWrite(rightA1, 255);
      analogWrite(rightA2, 255);
    break;
  }
}  




