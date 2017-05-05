/*Takes an input from the nunchuk and sends a control byte to the motors*/
#include <Wire.h>
#include <ArduinoNunchuk.h>

ArduinoNunchuk nunchuk = ArduinoNunchuk();

int leftCommand = 0;
int rightCommand = 0;

int leftA1 = 3;
int leftA2 = 4;
int rightA1 = 5;
int rightA2 = 6;

int incomingByte = 0;


/* Function declarations */
void setMotorCommands(int, int);
void controlMotors();
void parseByte(byte);


void setup() {
  Serial.begin(9600);
  //nunchuk.init();
  //Serial.print("nunchuk initialized");
}

void loop() {
//Output to LEDs when data is received
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    parseByte(incomingByte);
    controlMotors();
  }
}


void setMotorCommands(int left, int right) {
  leftCommand = left;
  rightCommand = right;
}

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

//parses leftCommand and rightCommand to send forward/reverse to each motor set
void controlMotors() {

    //clears input to each pin
    analogWrite(leftA1, 0);
    analogWrite(leftA2, 0);
    analogWrite(rightA1, 0);
    analogWrite(rightA2, 0); 
    

  switch(leftCommand) {
    case 0: // Left Wheel Backward Full -- Needs to go CCW
      analogWrite(leftA1, 0);
    break;
    case 1: 
      analogWrite(leftA1, 0);
      
    break;
    case 2:
      analogWrite(leftA1, 0);
      
    break;
    case 3:
      analogWrite(leftA1, 0);
      
    break;
    case 4:
      analogWrite(leftA1, 0);
      
    break;
    case 5:
      analogWrite(leftA1, 0);
      
    break;
    case 6: // Left Backward Low
      analogWrite(leftA1, 0);
      
    break;
    //case 7: NOTHING
      //analogWrite(leftBackwardLEDPin, 0);
    //break;
    case 8: // Left Wheel Forward Low -- Needs to go CW
      analogWrite(leftA1, 120);
      
    break;
    case 9:
      analogWrite(leftA1, 150);
      
    break;
    case 10:
      analogWrite(leftA1, 160);
      
    break;
    case 11:
      analogWrite(leftA1, 190);
      
    break;
    case 12:
      analogWrite(leftA1, 220);
     
    break;
    case 13:
      analogWrite(leftA1, 245);
     
    break;
    case 14: // Left Forward Full
      analogWrite(leftA1, 255);
      
    break;
  }

    switch(rightCommand) { // Right Wheel Set
    case 0: // Right Wheel Backward Full -- Needs to go CW
      analogWrite(rightA1, 0);
      
    break;
    case 1:
      analogWrite(rightA1, 0);
      
    break;
    case 2:
      analogWrite(rightA1, 0);
      
    break;
    case 3:
      analogWrite(rightA1, 0);
      
    break;
    case 4:
      analogWrite(rightA1, 0);
      
    break;
    case 5:
      analogWrite(rightA1, 0);
      
    break;
    case 6: // Right Backward 0
      analogWrite(rightA1, 0);
      
    break;
    case 7: 
      //analogWrite(rightBackwardLEDPin, 0);
      //analogWrite(rightA1, 255);
      //analogWrite(rightA2, 255);
    break;
    case 8: // Right Wheel Forward 0 -- Needs to go CCW
      analogWrite(rightA1, 120);
      
      
    break;
    case 9:
      analogWrite(rightA1, 150);

    break;
    case 10:
      analogWrite(rightA1, 160);
      
    break;
    case 11:
      analogWrite(rightA1, 190);
      
    break;
    case 12:
      analogWrite(rightA1, 220);
      
    case 13:
      analogWrite(rightA1, 245);
      
    case 14: // Right Forward Full
      analogWrite(rightA1, 255);
      analogWrite(rightA2, 120);
      
    break;
  }
}  




