const int jump = 1;
void setup() {
  pinMode(10, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly
  Serial.println("Beginning motor test");
  for (int i= 0; i < 255; i+=jump) {
    Serial.println(i);
    analogWrite(10, i);
    delay(3000);
  }
  for (int i= 255; i <= 0; i-=jump) {
    Serial.println(i);
    analogWrite(10, i);
    delay(3000);
  }
  //analogWrite(10,2);
  //delay(5000);


}
