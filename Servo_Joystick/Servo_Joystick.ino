#include <Servo.h>
#define servo 11
#define trigPin 5
#define echoPin 6
#define laser 8
#define buzzer 2
#define x_axis A0
#define y_axis A1

Servo servoMotor;
const int deadzone = 20;
const int center = 512;
float angle = 0;
void setup() {
  Serial.begin(9600);
  servoMotor.attach(servo);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(laser, OUTPUT);
  pinMode(buzzer, OUTPUT);
}

void loop() {
  long duration = 0;
  float distance = 0;

  for (angle = 0; angle <= 180; angle += 1) { 
    Serial.println(angle);
    servoMotor.write(angle);              // Tell servo to go to position in variable 'angle'
    delay(15);                         // Wait 15ms for the servo to reach the position
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    Serial.println(distance);
    if (distance < 20) {
      Serial.println("Detected");
      digitalWrite(laser, HIGH);
      tone(buzzer, 2000); 
      delay(80); // Play for a very brief 80 milliseconds
      
      // Turn the buzzer off
      noTone(buzzer);
      angle--;
    }
    
    digitalWrite(laser, LOW);
  }
  
  // Sweep from 180 back to 0 degrees
  for (angle = 180; angle >= 0; angle -= 1) { 
    servoMotor.write(angle);              // Tell servo to go to position in variable 'angle'
    delay(15);                         // Wait 15ms for the servo to reach the position
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    Serial.println(distance);
    if (distance < 20) {
      Serial.println("Detected");
      digitalWrite(laser, HIGH);
      tone(buzzer, 2000); 
      delay(80); // Play for a very brief 80 milliseconds
      
      // Turn the buzzer off
      noTone(buzzer);
      angle++;
    }
    
    digitalWrite(laser, LOW);
  }
  

}

