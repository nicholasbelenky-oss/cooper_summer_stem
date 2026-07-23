#include <Servo.h>
#define servo 11
#define x_axis A0
#define y_axis A1

Servo servoMotor;
int deadzone = 20;
int center = 512;
float angle = 0;
void setup() {
 
  Serial.begin(9600);
  servoMotor.attach(servo);
  servoMotor.write(30);
}

void loop() {
  int raw_x = analogRead(x_axis);
  int raw_y = analogRead(y_axis);
  
  int dx = raw_x - center;
  int dy = raw_y - center;
  
  if (abs(dx) < deadzone && abs(dy) < deadzone) {
    return;
  }
  
  float angle_rad = atan2(dy, dx);
  float angle_deg = angle_rad * 180.0 / PI; 
  
  if (angle_deg < 0) {
    angle_deg = -angle_deg;
  }

  servoMotor.write(angle_deg);
  Serial.println(angle_deg);
  
  delay(15); // Small delay to allow the servo to physically move
}

