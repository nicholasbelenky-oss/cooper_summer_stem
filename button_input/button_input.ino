#define button 7
#define LED 13

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(button) == LOW) {
    Serial.println(digitalRead(button));
    digitalWrite(LED, HIGH);
  }
  else {
    Serial.println(digitalRead(button));
    digitalWrite(LED, LOW);
  }

}
