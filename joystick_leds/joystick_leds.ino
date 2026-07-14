#define red_LED 3
#define yellow_LED 5
#define green_LED 6
#define blue_LED 9
#define x A0
#define y A1
#define switch_button 13

const int CENTER = 512;   // resting analogRead value for a centered joystick
const int DEADZONE = 20;  // ignore small noise/drift right around center

void setup() {
  pinMode(red_LED, OUTPUT);
  pinMode(yellow_LED, OUTPUT);
  pinMode(green_LED, OUTPUT);
  pinMode(blue_LED, OUTPUT);
  pinMode(switch_button, INPUT_PULLUP);
}

void loop() {
  while (digitalRead(switch_button) == LOW) {
    analogWrite(red_LED, 255);
    analogWrite(green_LED, 255);
    analogWrite(blue_LED, 255);
    analogWrite(yellow_LED, 255);
  }
  int raw_x = analogRead(x);
  int raw_y = analogRead(y);

  int dx = raw_x - CENTER;
  int dy = raw_y - CENTER;
  
  // Magnitude of the push, scaled down to a 0-255 brightness
  float distance = sqrt((float)dx * dx + (float)dy * dy);
  int power = (int)map((long)distance, 0, 512, 0, 255);
  power = constrain(power, 0, 255);

  // Reset all four every loop, then light only the active quadrant
  analogWrite(red_LED, 0);
  analogWrite(yellow_LED, 0);
  analogWrite(green_LED, 0);
  analogWrite(blue_LED, 0);
  dy = -dy;
  if (abs(dx) < DEADZONE && abs(dy) < DEADZONE) {
    return;
  }
  else if (abs(dx) < DEADZONE && dy >= 0) {
    analogWrite(red_LED, power);
    analogWrite(yellow_LED, power);
  }
  else if (abs(dx) < DEADZONE && dy < 0) {
    analogWrite(green_LED, power);
    analogWrite(blue_LED, power);
  }
  else if (abs(dy) < DEADZONE && dx >= 0) {
    analogWrite(red_LED, power);
    analogWrite(blue_LED, power);
  }
  else if (abs(dy) < DEADZONE && dx < 0) {
    analogWrite(green_LED, power);
    analogWrite(yellow_LED, power);
  }

  if (dx >= 0 && dy >= 0) {
    analogWrite(red_LED, power);
  } else if (dx < 0 && dy >= 0) {
    analogWrite(yellow_LED, power);
  } else if (dx < 0 && dy < 0) {
    analogWrite(green_LED, power);
  } else { 
    analogWrite(blue_LED, power);
  }
}
