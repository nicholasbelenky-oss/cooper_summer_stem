// Konami Code detector: Up, Up, Down, Down, Left, Right, Left, Right
// Joystick: VRx -> A0, VRy -> A1
// (SW pin is optional here -- included as a manual "reset progress" button)
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); 
const int VRX_PIN = A0;
const int VRY_PIN = A1;
const int SW_PIN  = 12;
const int buzzer = 3;
const int laser_1 = 10;
const int laser_2 = 9;
const int laser_3 = 8;

const int LED_PIN = LED_BUILTIN; // flashes on a successful entry, no extra wiring needed

int joyCenterX = 512;
int joyCenterY = 512;
const int JOY_DEADZONE = 150;

const unsigned long INPUT_TIMEOUT_MS = 2000; // reset progress if you pause too long
const unsigned long DEBOUNCE_MS = 30;        // ignores tiny analog jitter near the threshold

enum Dir { DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

const Dir KONAMI_SEQUENCE[] = {
  DIR_UP, DIR_UP, DIR_DOWN, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_LEFT, DIR_RIGHT
};
const int SEQUENCE_LEN = sizeof(KONAMI_SEQUENCE) / sizeof(KONAMI_SEQUENCE[0]);

int matchIndex = 0;
Dir lastDir = DIR_NONE;          // last direction we already counted as an "event"
Dir lastStableReading = DIR_NONE; // for simple debounce
unsigned long lastReadingChangeTime = 0;
unsigned long lastInputTime = 0;

void setup() {
  Serial.begin(9600);
  lcd.init();                     
  lcd.backlight();
  lcd.setCursor(0, 0);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(buzzer, OUTPUT);
  pinMode(laser_1, OUTPUT);
  pinMode(laser_2, OUTPUT);
  pinMode(laser_3, OUTPUT);
  digitalWrite(laser_1, HIGH);
  digitalWrite(laser_2, HIGH);
  digitalWrite(laser_3, HIGH);


  calibrateJoystick();

  Serial.println("Konami code detector ready.");
  lcd.print("Enter code");
}

void loop() {
  // Manual reset button, handy for testing
  if (digitalRead(SW_PIN) == LOW) {
    resetProgress("Manual reset");
    delay(200); // crude debounce, fine for a reset button
  }

  Dir reading = readJoystickDirection();

  // Simple debounce: only trust a reading once it's held steady briefly
  if (reading != lastStableReading) {
    lastReadingChangeTime = millis();
    lastStableReading = reading;
  }
  bool stable = (millis() - lastReadingChangeTime) > DEBOUNCE_MS;

  if (stable && reading != lastDir) {
    lastDir = reading;
    if (reading != DIR_NONE) {
      onDirectionEntered(reading);
    }
  }

  // Timed out waiting for the next input in the sequence
  if (matchIndex > 0 && (millis() - lastInputTime) > INPUT_TIMEOUT_MS) {
    resetProgress("Timed out");
  }
}

Dir readJoystickDirection() {
  int rx = analogRead(VRX_PIN);
  int ry = analogRead(VRY_PIN);

  int dx = rx - joyCenterX;
  int dy = ry - joyCenterY;

  // If directions feel swapped/inverted on your hardware, swap rx/ry above,
  // or flip the comparison signs below.
  if (abs(dx) > abs(dy)) {
    if (dx > JOY_DEADZONE) return DIR_RIGHT;
    if (dx < -JOY_DEADZONE) return DIR_LEFT;
  } else {
    if (dy > JOY_DEADZONE) return DIR_DOWN;
    if (dy < -JOY_DEADZONE) return DIR_UP;
  }
  return DIR_NONE;
}
void IncorrectBuzzer() {
  tone(buzzer, 250, 300); // Play a low 250Hz tone for 300ms
  delay(350);                 // Wait for it to finish + a tiny pause
  
  tone(buzzer, 200, 500); // Play an even lower 200Hz tone for 500ms
  delay(500);
}
void SuccessSound() {
  // A rapid, cheerful rising chord (C5 -> E5 -> G5 -> C6)
  
  tone(buzzer, 523, 100);  // Play C5 for 100ms
  delay(120);                  // Wait for the note to finish + a tiny gap
  
  tone(buzzer, 659, 100);  // Play E5 for 100ms
  delay(120);
  
  tone(buzzer, 784, 100);  // Play G5 for 100ms
  delay(120);
  
  tone(buzzer, 1047, 250); // Play high C6 for 250ms (the big finish!)
  delay(270);
}

void onDirectionEntered(Dir d) {
  lastInputTime = millis();
  Serial.print("Input: ");
  Serial.println(dirName(d));
  lcd.clear();
  lcd.print("Input: ");
  lcd.print(dirName(d));

  if (d == KONAMI_SEQUENCE[matchIndex]) {
    matchIndex++;
    Serial.print("  -> progress: ");
    Serial.print(matchIndex);
    Serial.print("/");
    Serial.println(SEQUENCE_LEN);
    lcd.clear();
    lcd.print("-> progress: ");
    lcd.print(matchIndex);
    lcd.print("/");
    lcd.println(SEQUENCE_LEN);

    if (matchIndex >= SEQUENCE_LEN) {
      onKonamiCodeSuccess();
    }
  } else {
    // Wrong input -- but it might be the correct START of a new attempt,
    // so check that instead of just resetting to zero blindly.
    if (d == KONAMI_SEQUENCE[0]) {
      matchIndex = 1;
      Serial.println("  -> wrong, but restarting sequence from this input (1/8)");
      lcd.clear();
      lcd.println("  -> wrong, but restarting sequence from this input (1/8)");
    } else {
      matchIndex = 0;
      Serial.println("  -> wrong, sequence reset");
      lcd.clear();
      lcd.println("-> wrong, reset");
      IncorrectBuzzer();

    }
  }
}

void onKonamiCodeSuccess() {
  Serial.println("*** KONAMI CODE ENTERED! ***");
  lcd.clear();
  lcd.println("*** KONAMI CODE ENTERED! ***");
  for (int i = 0; i < 6; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  matchIndex = 0;
  digitalWrite(laser_1, LOW);
  digitalWrite(laser_2, LOW);
  digitalWrite(laser_3, LOW);
  SuccessSound();
}

void resetProgress(const char* reason) {
  if (matchIndex != 0) {
    Serial.print("Progress reset (");
    Serial.print(reason);
    Serial.println(")");
    lcd.clear();
    lcd.print("Reset (");
    lcd.print(reason);
    lcd.println(")");
    IncorrectBuzzer();
    
  }
  matchIndex = 0;
}

const char* dirName(Dir d) {
  switch (d) {
    case DIR_UP:    return "UP";
    case DIR_DOWN:  return "DOWN";
    case DIR_LEFT:  return "LEFT";
    case DIR_RIGHT: return "RIGHT";
    default:        return "NONE";
  }
}

// Reads the joystick's resting position a few times at boot and uses that as
// "center" instead of assuming a hardcoded 512. Don't touch the stick while
// this runs (right at power-on / reset).
void calibrateJoystick() {
  long sumX = 0, sumY = 0;
  const int samples = 20;
  for (int i = 0; i < samples; i++) {
    sumX += analogRead(VRX_PIN);
    sumY += analogRead(VRY_PIN);
    delay(5);
  }
  joyCenterX = sumX / samples;
  joyCenterY = sumY / samples;

  Serial.print("Joystick calibrated. Center X=");
  Serial.print(joyCenterX);
  Serial.print(" Y=");
  Serial.println(joyCenterY);
}
