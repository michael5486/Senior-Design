int leftForwardLEDPin = 10;       // LED connected to digital pin 9
int leftBackwardLEDPin = 9;    // LED connected to digital pin 10
int rightForwardLEDPin = 6;     // LED connected to digital pin 5
int rightBackwardLEDPin = 5;    // LED connected to digital pin 6


int incomingByte = 0;

int leftLEDCommand = 0;
int rightLEDCommand = 0;

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

  leftLEDCommand = tempLeft;
  rightLEDCommand = tempRight;
}

//parses leftLEDCommand and rightLEDCommand to send forward/reverse to each LED
//two LEDs for each motor, for forward and reverse
void controlLEDs() {

    //clears input to each pin
    analogWrite(leftForwardLEDPin, 0);
    analogWrite(leftBackwardLEDPin, 0);
    analogWrite(rightForwardLEDPin, 0);
    analogWrite(rightBackwardLEDPin, 0);

  switch(leftLEDCommand) {
    case 0:
      analogWrite(leftBackwardLEDPin, 252);
    break;
    case 1:
      analogWrite(leftBackwardLEDPin, 216);
    break;
    case 2:
      analogWrite(leftBackwardLEDPin, 180);
    break;
    case 3:
      analogWrite(leftBackwardLEDPin, 144);
    break;
    case 4:
      analogWrite(leftBackwardLEDPin, 108);
    break;
    case 5:
      analogWrite(leftBackwardLEDPin, 72);
    break;
    case 6:
      analogWrite(leftBackwardLEDPin, 36);
    break;
    //case 7:
      //analogWrite(leftBackwardLEDPin, 0);
    //break;
    case 8:
      analogWrite(leftForwardLEDPin, 36);
    break;
    case 9:
      analogWrite(leftForwardLEDPin, 72);
    break;
    case 10:
      analogWrite(leftForwardLEDPin, 108);
    break;
    case 11:
      analogWrite(leftForwardLEDPin, 144);
    break;
    case 12:
      analogWrite(leftForwardLEDPin, 180);
    break;
    case 13:
      analogWrite(leftForwardLEDPin, 216);
    break;
    case 14:
      analogWrite(leftForwardLEDPin, 252);
    break;
  }

    switch(rightLEDCommand) {
    case 0:
      analogWrite(rightBackwardLEDPin, 252);
    break;
    case 1:
      analogWrite(rightBackwardLEDPin, 216);
    break;
    case 2:
      analogWrite(rightBackwardLEDPin, 180);
    break;
    case 3:
      analogWrite(rightBackwardLEDPin, 144);
    break;
    case 4:
      analogWrite(rightBackwardLEDPin, 108);
    break;
    case 5:
      analogWrite(rightBackwardLEDPin, 72);
    break;
    case 6:
      analogWrite(rightBackwardLEDPin, 36);
    break;
    //case 7:
      //analogWrite(rightBackwardLEDPin, 0);
    //break;
    case 8:
      analogWrite(rightForwardLEDPin, 36);
    break;
    case 9:
      analogWrite(rightForwardLEDPin, 72);
    break;
    case 10:
      analogWrite(rightForwardLEDPin, 108);
    break;
    case 11:
      analogWrite(rightForwardLEDPin, 144);
    break;
    case 12:
      analogWrite(rightForwardLEDPin, 180);
    break;
    case 13:
      analogWrite(rightForwardLEDPin, 216);
    break;
    case 14:
      analogWrite(rightForwardLEDPin, 252);
    break;
  }
}  




