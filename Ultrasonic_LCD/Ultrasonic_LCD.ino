#define trigPin 12
#define echoPin 13
#define button 6
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(button, INPUT_PULLUP); // Active LOW (LOW when pressed)
  Serial.begin(9600);
  
  lcd.init();                     
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sensing...");
  delay(1000);
}

void loop() {
  randomSeed(analogRead(A0));
  
  // Target distance between 5 and 25 cm (0-5cm is too close for HC-SR04)
  int random_distance = random(5, 25); 
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Target: ");
  lcd.print(random_distance);
  lcd.print(" cm");
  lcd.setCursor(0, 1);
  lcd.print("Press to lock in");

  // 1. Wait for the button to be pressed (goes LOW when pressed)
  while (digitalRead(button) == HIGH) {
    delay(10); // Small delay to keep the microcontroller stable
  }
  
  // Debounce delay to prevent accidental double-registers
  delay(50);

  // 2. Measure the distance EXACTLY at the moment the button is pressed
  long duration = 0;
  float distance = 0;
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Handle out-of-range sensor readings
  if (distance == 0 || distance > 400) {
    lcd.clear();
    lcd.print("Sensor Error!");
    lcd.setCursor(0, 1);
    lcd.print("Try again.");
    delay(2000);
    return; // Restart the loop
  }

  // Calculate the difference
  float difference = abs(distance - random_distance);

  // 3. Display the results
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("You: ");
  lcd.print(distance, 1); // Prints with 1 decimal place
  lcd.print(" cm");
  
  lcd.setCursor(0, 1);
  if (difference >= 5.0) {
    lcd.print("U suck! Diff: ");
    lcd.print(difference, 1);
  } 
  else if (difference > 0.5) { // Close (within 5cm but not perfect)
    lcd.print("Close! Diff: ");
    lcd.print(difference, 1);
  } 
  else { // "On the dot" is now anything within a reasonable 0.5 cm
    lcd.print("Perfect score!");
  }
  
  delay(3000); // Give them time to read their score

  // 4. Wait for the user to RELEASE the button before prompting reset
  while (digitalRead(button) == LOW) {
    delay(10);
  }

  // 5. Prompt for reset
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button");
  lcd.setCursor(0, 1);
  lcd.print("to restart...");

  // Wait for button press to restart
  while (digitalRead(button) == HIGH) {
    delay(10);
  }

  // Wait for button release again so the next round doesn't instantly trigger
  while (digitalRead(button) == LOW) {
    delay(10);
  }
  
  delay(200); // Brief pause before starting the next round
}