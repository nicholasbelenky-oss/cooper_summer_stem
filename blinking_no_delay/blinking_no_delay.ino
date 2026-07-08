#define LED 13
#define interval 50
unsigned long time = millis();
unsigned long previous = 0;
int ledState = LOW;

void setup() {
  pinMode(LED, OUTPUT);
}

void loop() {
  time = millis();
  if (time - previous > interval) {
    previous = time;
    if (ledState == LOW) {
      ledState = HIGH;
    }
    else {
      ledState = LOW;
    }
  }
  digitalWrite(LED, ledState);

}
