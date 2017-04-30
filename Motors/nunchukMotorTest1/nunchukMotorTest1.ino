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

/* Different Set of H-Bridges L6203 */
/* SET PINS HERE MIKEY */
int leftB1 = ;
int leftB2 = ;
int rightB1 = ;
int rightB2 = ;

/* ENABLE PIN NEEDS TO BE TURNED ON FOR THESE H BRIDGES TO WORK, IDK WHERE TO IMPLEMENT DIGITAL PINOUT */
int enabler = ;

/* Function declarations */
void setMotorCommands(int, int);
void controlMotors();


void setup() {
  Serial.begin(9600);
  nunchuk.init();
  Serial.print("nunchuk initialized");
}

void loop() {
  nunchuk.update();
  if (nunchuk.analogX < 50) {
    Serial.print("going left\n");
    setMotorCommands(0, 14);
    controlMotors();
  }
  else if (nunchuk.analogX > 200) {
    Serial.print("going right\n");
    setMotorCommands(14, 0);
    controlMotors();
  }
  else if (nunchuk.analogY > 200) {
    Serial.print("going forward\n");
    setMotorCommands(14, 14);
    controlMotors();
  }
  else if (nunchuk.analogY < 50) {
    Serial.print("going backward\n");
    setMotorCommands(0, 0);
    controlMotors();
  }
  else {
    Serial.print("stop moving\n");
    setMotorCommands(7 ,7);
    controlMotors();
  }
}


void setMotorCommands(int left, int right) {
  leftCommand = left;
  rightCommand = right;
}

//parses leftCommand and rightCommand to send forward/reverse to each motor set
void controlMotors() {

    //clears input to each pin
    analogWrite(leftA1, 255);
    analogWrite(leftA2, 255);
    analogWrite(rightA1, 255);
    analogWrite(rightA2, 255);

     /* For L6203, set inputs 1 and 2 to L for off */
     analogWrite(leftB1, 0);
    analogWrite(leftB2, 0);
    analogWrite(rightB1, 0);
    analogWrite(rightB2, 0);
    
/* Input settings for each set of H-bridges given desired motor output */
/* Need to experiment to determine what speeds correspond to the correct */

  switch(leftCommand) {
    case 0: // Left Wheel Backward Full -- Needs to go CCW
      analogWrite(leftA1, 0);
      analogWrite(leftA2, 255);
      
      analogWrite(leftB1, 0);
      analogWrite(leftB2, 255);
    break;
    case 1: 
      analogWrite(leftA1, 30);
      analogWrite(leftA2, 255);

      analogWrite(leftB1, 0);
      analogWrite(leftB2, 225);
    break;
    case 2:
      analogWrite(leftA1, 60);
      analogWrite(leftA2, 255);

      analogWrite(leftB1, 0);
      analogWrite(leftB2, 195);
    break;
    case 3:
      analogWrite(leftA1, 90);
      analogWrite(leftA2, 255);

      analogWrite(leftB1, 0);
      analogWrite(leftB2, 165);
    break;
    case 4:
      analogWrite(leftA1, 120);
      analogWrite(leftA2, 255);

      analogWrite(leftB1, 0);
      analogWrite(leftB2, 135);
    break;
    case 5:
      analogWrite(leftA1, 150);
      analogWrite(leftA2, 255);

      analogWrite(leftB1, 0);
      analogWrite(leftB2, 105);
    break;
    case 6: // Left Backward Low
      analogWrite(leftA1, 180);
      analogWrite(leftA2, 255);

      analogWrite(leftB1, 0);
      analogWrite(leftB2, 75);
    break;
    //case 7: No movement
      analogWrite(leftA1, 255);
      analogWrite(leftA2, 255);

      analogWrite(leftB1, 0);
      analogWrite(leftB2, 0);
    //break;
    case 8: // Left Wheel Forward Low -- Needs to go CW
      analogWrite(leftA1, 180);
      analogWrite(leftA2, 180);

      analogWrite(leftB1, 75);
      analogWrite(leftB2, 0);
    break;
    case 9:
      analogWrite(leftA1, 150);
      analogWrite(leftA2, 150);

      analogWrite(leftB1, 105);
      analogWrite(leftB2, 0);
    break;
    case 10:
      analogWrite(leftA1, 120);
      analogWrite(leftA2, 120);

      analogWrite(leftB1, 135);
      analogWrite(leftB2, 0);
    break;
    case 11:
      analogWrite(leftA1, 90);
      analogWrite(leftA2, 90);

      analogWrite(leftB1, 165);
      analogWrite(leftB2, 0);
    break;
    case 12:
      analogWrite(leftA1, 60);
      analogWrite(leftA2, 60);

      analogWrite(leftB1, 195);
      analogWrite(leftB2, 0);
    break;
    case 13:
      analogWrite(leftA1, 30);
      analogWrite(leftA2, 30);

      analogWrite(leftB1, 225);
      analogWrite(leftB2, 0);
    break;
    case 14: // Left Forward Full
      analogWrite(leftA1, 0);
      analogWrite(leftA2, 0);

      analogWrite(leftB1, 255);
      analogWrite(leftB2, 0);
    break;
  }

    switch(rightCommand) { // Right Wheel Set
    case 0: // Right Wheel Backward Full -- Needs to go CW
      analogWrite(rightA1, 0);
      analogWrite(rightA2, 0);

      analogWrite(rightB1, 255);
      analogWrite(rightB2, 0);
    break;
    case 1:
      analogWrite(rightA1, 30);
      analogWrite(rightA2, 30);

      analogWrite(rightB1, 225);
      analogWrite(rightB2, 0);
    break;
    case 2:
      analogWrite(rightA1, 60);
      analogWrite(rightA2, 60);

      analogWrite(rightB1, 195);
      analogWrite(rightB2, 0);
    break;
    case 3:
      analogWrite(rightA1, 90);
      analogWrite(rightA2, 90);

      analogWrite(rightB1, 165);
      analogWrite(rightB2, 0);
    break;
    case 4:
      analogWrite(rightA1, 120);
      analogWrite(rightA2, 120);

      analogWrite(rightB1, 135);
      analogWrite(rightB2, 0);
    break;
    case 5:
      analogWrite(rightA1, 150);
      analogWrite(rightA2, 150);

      analogWrite(rightB1, 105);
      analogWrite(rightB2, 0);
    break;
    case 6: // Right Backward 0
      analogWrite(rightA1, 180);
      analogWrite(rightA2, 180);

      analogWrite(rightB1, 75);
      analogWrite(rightB2, 0);
    break;
    case 7: 
      //analogWrite(rightBackwardLEDPin, 0);
      analogWrite(rightA1, 255);
      analogWrite(rightA2, 255);

      analogWrite(rightB1, 0);
      analogWrite(rightB2, 0);
    break;
    case 8: // Right Wheel Forward 0 -- Needs to go CCW
      analogWrite(rightA1, 180);
      analogWrite(rightA2, 255);

      analogWrite(rightB1, 0);
      analogWrite(rightB2, 75);
    break;
    case 9:
      analogWrite(rightA1, 150);
      analogWrite(rightA2, 255);

      analogWrite(rightB1, 0);
      analogWrite(rightB2, 105);
    break;
    case 10:
      analogWrite(rightA1, 120);
      analogWrite(rightA2, 255);

      analogWrite(rightB1, 0);
      analogWrite(rightB2, 135);
    break;
    case 11:
      analogWrite(rightA1, 90);
      analogWrite(rightA2, 255);

      analogWrite(rightB1, 0);
      analogWrite(rightB2, 165);
    break;
    case 12:
      analogWrite(rightA1, 60);
      analogWrite(rightA2, 255);

      analogWrite(rightB1, 0);
      analogWrite(rightB2, 195);
    case 13:
      analogWrite(rightA1, 30);
      analogWrite(rightA2, 255);

      analogWrite(rightB1, 0);
      analogWrite(rightB2, 225);
    case 14: // Right Forward Full
      analogWrite(rightA1, 0);
      analogWrite(rightA2, 255);

      analogWrite(rightB1, 0);
      analogWrite(rightB2, 255);
    break;
  }
}  




