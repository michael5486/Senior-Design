#include <Servo.h>

int pos = 0;
int buttonPin = 2;
int servoPin = 9;
volatile int buttonPressed = LOW;

Servo myServo;

void setup() {
  // put your setup code here, to run once:
  myServo.attach(servoPin);
  attachInterrupt(digitalPinToInterrupt(buttonPin), pressed, RISING);
  myServo.write(5);
}

void loop() {
  // put your main code here, to run repeatedly:
  pos = myServo.read();
  if(buttonPressed == HIGH && pos == 125)
  {
    myServo.write(5);
    delay(1000);
    buttonPressed = LOW;
  }
  
  if(buttonPressed == HIGH && pos == 5) 
  {
    myServo.write(125);
    delay(1000);
    buttonPressed = LOW;
  }
  
}
void pressed() {
  buttonPressed = HIGH;
}

