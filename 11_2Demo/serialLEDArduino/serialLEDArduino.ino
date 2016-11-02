//code to get motor control started with OpenCV


const int ledPin1 = 13; // the pin that the LED is attached to
const int ledPin2 = 12;
int incomingByte;      // a variable to read incoming serial data into
int val1 = 0;
int val2 = 0;

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  // initialize the LED pins as an output:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);


}

void loop() {
  // see if there's incoming serial data:
  if (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    incomingByte = Serial.read();
    // if it's an L, turn toggle LED1:
    if (incomingByte == 'L') {
      if (val1 == 0) {
        digitalWrite(ledPin1, HIGH);
        val1 = 1;
      }
      else {
        digitalWrite(ledPin1, LOW);
        val1 = 0;
        Serial.write("LOW");

      }
      Serial.write(val1);

    }
    // if it's an R, toggle LED2:
    if (incomingByte == 'R') {
       if (val2 == 0) {
        digitalWrite(ledPin2, HIGH);
        val2 = 1;
        Serial.write("HIGH");
      }
      else {
        digitalWrite(ledPin2, LOW);
        val2 = 0;
        Serial.write("LOW");
      }
    }
  }
}
