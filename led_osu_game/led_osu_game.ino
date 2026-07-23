#include "merry_christmas.h"
#include "furelise.h"
#include "harry_p.h"
#include "tetris.h"
// include song libraries

#include <SoftPWM.h>
//include analog -> PWM converter

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
// lcd inclusions


// change this to whichever pin you want to use
int buzzer = A5;


// change this to make the song slower or faster
int christmas_tempo = 160;
int furelise_tempo = 80;
int harry_p_tempo = 144;
int tetris_tempo = 144;

int tempo = christmas_tempo;


// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(christmas_melody) / sizeof(christmas_melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;


// 3x3 Pin Layout Grid mapping directly to your hardware
const int ledGrid[3][3] = {
  {7, 10, 13},  // Top Row (Left, Center, Right)
  {6,  9, 12},  // Middle Row (Left, Center, Right)
  {5,  8, 11},  
  {14, 15, 16}   // Bottom Row (Left, Center, Right)
};

// Buttons: index 0 = Left column, 1 = Center column, 2 = Right column
// const int buttonList[] = {2, 3, 4};
const int joystick_x = 2;
const int joystick_switch = 3;

// Game Timing Configuration
const int bpm = 160;
const unsigned long beatInterval = 60000 / bpm; // ms per beat

// LED State Tracking (0 = Off, 1 = On)
int matrix[3][3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};

// Sequencer: Defines which column gets a new note on each beat
// 0 = Left, 1 = Center, 2 = Right, -1 = Empty Beat
const int trackLength = 29;
const int noteSequence[trackLength] = {
 0, 1, 1, 2, 1, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 2, 1, 2, 1, 0, 0, 0, 1, 0, 2, 0, 1
};

int sequenceIndex = 0;
unsigned long lastBeatTime = 0;

// ---------- Button / scoring state ----------
// lastRawState: the raw digitalRead() from the previous poll, used only to
//               notice when the pin is flickering so we can reset the timer.
// buttonState:  the CONFIRMED, debounced state -- this is what we compare
//               against to detect an actual press edge.
int lastRawState[3] = {HIGH, HIGH, HIGH};
int buttonState[3] = {HIGH, HIGH, HIGH};
unsigned long lastDebounceTime[3] = {0, 0, 0};
const unsigned long debounceDelay = 15; // ms

int score = 0;
int hits = 0;
int misses = 0;
int wrongPresses = 0;

void setup() {
  // initialize serial monitor
  Serial.begin(9600);

  // intialize lcd 
  lcd.init();                     
  lcd.backlight();
  lcd.setCursor(0, 0);

  // Initialize the SoftPWM library
  SoftPWMInitialize();
  
  // Set all 9 mapped pins as OUTPUT and turn them off initially
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      pinMode(ledGrid[row][col], OUTPUT);
      digitalWrite(ledGrid[row][col], LOW);
    }
  }
  for (int col = 0; col < 3; col++) {
    SoftPWMSetFadeTime(ledGrid[3][col], 0, 0);
    SoftPWMSet(ledGrid[3][col], 0);
  }
  // Buttons wired to GND, using internal pullups
  // for (int i = 0; i < 3; i++) {
  //   pinMode(buttonList[i], INPUT_PULLUP);
  // }
  pinMode(change_song, INPUT_PULLUP);
  playSong();

  // Song's over - print final score
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      digitalWrite(ledGrid[row][col], HIGH);
  Serial.println("=== Song complete ===");
  Serial.print("Score: "); Serial.println(score);
  Serial.print("Hits: "); Serial.println(hits);
  Serial.print("Misses: "); Serial.println(misses);
  Serial.print("Wrong presses: "); Serial.println(wrongPresses);

  lcd.print("=== Song complete ===");
  delay(1000);
  lcd.clear();
  lcd.print("Score: "); lcd.print(score);
  delay(1000);
  lcd.clear();
  lcd.print("Hits: "); lcd.print(hits);
  lcd.print("Misses: "); lcd.print(misses);
  lcd.setCursor(1, 0);
  lcd.print("Wrong presses: "); lcd.println(wrongPresses);
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Click joystick to play again!");
}
  }
}

void playSong() {
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = christmas_melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, christmas_melody[thisNote], noteDuration * 0.9);

    // Wait for the note's duration, but keep polling buttons the whole time
    // instead of blind-delaying, so presses during this beat aren't missed.
    waitAndCheckButtons(noteDuration);

    // Any note still lit in the bottom row survived the whole beat unhit -> miss
    registerMissedNotes();

    advanceGameClock();
    refreshDisplay();

    // stop the waveform generation before the next note.
    noTone(buzzer);
  }
}

// Waits `duration` ms, polling the buttons continuously instead of using delay().
void waitAndCheckButtons(unsigned long duration) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    checkButtons();
  }
}

// Reads all three buttons with proper debounce and fires a hit/miss check
// on each new confirmed press (HIGH -> LOW transition, INPUT_PULLUP wiring).
void checkButtons() {
  for (int i = 0; i < 3; i++) {
    int reading = digitalRead(buttonList[i]);

    // If the raw pin state changed since last poll (even due to bounce),
    // restart the debounce timer.
    if (reading != lastRawState[i]) {
      lastDebounceTime[i] = millis();
    }

    // Only trust the reading once it's been stable for debounceDelay ms
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      // Has the CONFIRMED state actually changed?
      if (reading != buttonState[i]) {
        buttonState[i] = reading;
        if (buttonState[i] == LOW) {
          // fresh, debounce-confirmed press
          onButtonPressed(i);
        }
      }
    }

    lastRawState[i] = reading;
  }
}

// Called the instant a button for column `col` is pressed.
void onButtonPressed(int col) {
  if (matrix[2][col] == 1) {
    // Hit! Note was in the bottom row for this column.
    hits++;
    score += 100;
    matrix[2][col] = 0;   // clear it so it isn't double-counted or double-scored
    refreshDisplay();     // immediate visual feedback: LED turns off right away
    Serial.print("HIT col "); Serial.print(col); Serial.print(" score="); Serial.println(score);
  } else {
    // Pressed with nothing to hit in that column right now
    wrongPresses++;
    Serial.print("MISS PRESS col "); Serial.println(col);
  }
}

// Any note still lit in the bottom row when the beat ends went unhit.
void registerMissedNotes() {
  for (int col = 0; col < 3; col++) {
    if (matrix[2][col] == 1) {
      misses++;
      Serial.print("NOTE MISSED col "); Serial.println(col);
    }
  }
}

void advanceGameClock() {
  // 1. Shift existing notes DOWNWARDS
  // Row 1 (middle) drops down to Row 2 (bottom)
  // Row 0 (top) drops down to Row 1 (middle)
  for (int col = 0; col < 3; col++) {
    matrix[2][col] = matrix[1][col]; // Move middle to bottom
    matrix[1][col] = matrix[0][col]; // Move top to middle
    matrix[0][col] = 0;              // Clear top row for the new note
  }

  // 2. Spawn a new note on the top row (Row 0) from our track sequence
  int currentNoteColumn = random(3);
  if (currentNoteColumn >= 0 && currentNoteColumn <= 2) {
    matrix[0][currentNoteColumn] = 1;
  }

  // 3. Move to the next beat in the song array
  sequenceIndex++;
  if (sequenceIndex >= trackLength) {
    sequenceIndex = 0; // Loop song infinitely
  }
}

// Directly writes state to all 9 individual pins
void refreshDisplay() {
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      if (matrix[row][col] == 1) {
        digitalWrite(ledGrid[row][col], HIGH); // Turn LED On
      } else {
        digitalWrite(ledGrid[row][col], LOW);  // Turn LED Off
      }
    }
  }
}

void loop() {

}
