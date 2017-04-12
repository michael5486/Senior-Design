/* Sketch to demo control input from the nunchuk in 4 directions 
   Can add more directions if needed */

#include <Wire.h>
#include <ArduinoNunchuk.h>

ArduinoNunchuk nunchuk = ArduinoNunchuk();

void setup() {
  Serial.begin(9600);
  nunchuk.init();
  Serial.print("nunchuk initialized\n");
  
}

void loop() {
  nunchuk.update();

  if (nunchuk.analogX < 50) {
    Serial.print("going left\n");
  }
  else if (nunchuk.analogX > 200) {
    Serial.print("going right\n");
  }
  else if (nunchuk.analogY > 200) {
    Serial.print("going forward\n");
  }
  else if (nunchuk.analogY < 50) {
    Serial.print("going backward\n");
  }

  delay(50);

}
