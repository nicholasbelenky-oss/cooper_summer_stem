#include <NewTone.h>

#include "merry_christmas.h"   // expected to define: int christmas_melody[]
#include "furelise.h"          // expected to define: int furelise_melody[]
#include "harry_p.h"           // expected to define: int harry_p_melody[]
#include "tetris.h"            // expected to define: int tetris_melody[]
#include "Imperial_march.h"
// If your headers use different array names, just fix the songs[] list below.

#include <SoftPWM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- Pins ----------------
// Restart is handled by a physical button wired straight to the RESET pin +
// GND -- no GPIO pin needed, no code needed for that part.
const int buzzer            = 7;
const int JOYSTICK_SW_PIN   = 13;
const int JOYSTICK_X_PIN    = A1;  // moved off A5 -- A5 is the LCD's I2C SCL line

// ---------------- Grid geometry ----------------
const int NUM_ROWS = 4;
const int NUM_COLS = 3;
const int BOTTOM_ROW = NUM_ROWS - 1;

const int ledGrid[NUM_ROWS][NUM_COLS] = {
  {12,  8, 10},
  { 5,  6, 14},
  {16,  3, 17},
  { 2,  9,  4}
};

const int FADE_IN_MS  = 120;  // SoftPWM ramp-up time, for a smoother "note arriving" glow
const int FADE_OUT_MS = 200;  // SoftPWM ramp-down time, for a smoother "note leaving" fade

int matrix[NUM_ROWS][NUM_COLS];

// ---------------- Songs ----------------
struct Song {
  const char* name;
  const int* melody;
  int noteCount; // pitch+duration pairs, so this is melody length / 2
  int tempo;
};

int christmas_tempo = 160;
int furelise_tempo   = 80;
int harry_p_tempo    = 144;
int tetris_tempo     = 144;
int imperial_march_tempo = 120;

Song songs[] = {
  {"Christmas",     christmas_melody, sizeof(christmas_melody) / sizeof(christmas_melody[0]) / 2, christmas_tempo},
  {"Fur Elise",     furelise_melody,  sizeof(furelise_melody)  / sizeof(furelise_melody[0])  / 2, furelise_tempo},
  {"Harry Potter",  harry_p_melody,   sizeof(harry_p_melody)   / sizeof(harry_p_melody[0])   / 2, harry_p_tempo},
  {"Tetris",        tetris_melody,    sizeof(tetris_melody)    / sizeof(tetris_melody[0])    / 2, tetris_tempo},
  {"Imperial March", imperial_march_melody, sizeof(imperial_march_melody) / sizeof(imperial_march_melody[0]) / 2, imperial_march_tempo}
};
const int NUM_SONGS = sizeof(songs) / sizeof(songs[0]);

// ---------------- Game state ----------------
int score = 0;
int hits = 0;
int misses = 0;
int wrongPresses = 0;

// Debounce state for the joystick click (two-variable pattern: raw reading
// vs. confirmed state, so a press only fires once per press).
int swRawLast = HIGH;
int swConfirmed = HIGH;
unsigned long swDebounceStart = 0;
const unsigned long debounceDelay = 15; // ms

void setup() {
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();

  SoftPWMBegin();

  for (int r = 0; r < NUM_ROWS; r++) {
    for (int c = 0; c < NUM_COLS; c++) {
      SoftPWMSetFadeTime(ledGrid[r][c], FADE_IN_MS, FADE_OUT_MS);
      SoftPWMSet(ledGrid[r][c], 0);
    }
  }

  pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP);

  // No analog pin is free to use as a floating noise source, so seed off
  // millis() plus a live sensor reading instead.
  randomSeed(millis() + analogRead(JOYSTICK_X_PIN));

  runGameForever();
}

void loop() {
  // Everything happens inside runGameForever(), called once from setup().
}

// ---------------- Top-level flow ----------------

void runGameForever() {
  while (true) {
    int songIndex = selectSongMenu();
    resetStatsAndMatrix();
    playSong(songs[songIndex]);
    showEndScreens();
    // Loop back around -> selectSongMenu() shows its own prompt immediately.
    // This IS the "return to song select after the song ends" behavior.
  }
}

// ---------------- Song select menu ----------------

int selectSongMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Scroll: song"));

  // Clear any stale debounce/edge state so leftovers don't insta-trigger.
  swConfirmed = digitalRead(JOYSTICK_SW_PIN);
  swRawLast = swConfirmed;

  static int songIndex = 0; // remembers where you left off between plays
  int lastXDir = 0;         // requires returning to center before repeating a scroll step
  showSongName(songIndex);

  while (true) {
    int raw = analogRead(JOYSTICK_X_PIN);
    Serial.print(raw);
    Serial.print(", ");
    int dir = 0;
    if (raw < 512 - 200) dir = -1;      // pushed left
    else if (raw > 512 + 200) dir = 1;  // pushed right
    Serial.println(dir);
    if (dir != 0 && dir != lastXDir) {
      songIndex = (songIndex + dir + NUM_SONGS) % NUM_SONGS;
      showSongName(songIndex);
    }
    lastXDir = dir;

    if (joystickClickEdge()) {
      return songIndex;
    }
  }
}

void showSongName(int idx) {
  lcd.setCursor(0, 1);
  lcd.print(F("                ")); // 16 spaces, clears the row
  lcd.setCursor(0, 1);
  lcd.print(songs[idx].name);
}

// ---------------- Playing a song ----------------

void resetStatsAndMatrix() {
  score = 0;
  hits = 0;
  misses = 0;
  wrongPresses = 0;
  for (int r = 0; r < NUM_ROWS; r++) {
    for (int c = 0; c < NUM_COLS; c++) {
      matrix[r][c] = 0;
    }
  }
  refreshDisplay();
}

void playSong(const Song &song) {
  int wholenote = (60000 * 4) / song.tempo;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(song.name);
  updateLiveScore();

  for (int thisNote = 0; thisNote < song.noteCount * 2; thisNote += 2) {
    
    // Cast to signed 16-bit integers from Flash
    int currentPitch = (int16_t)pgm_read_word(&(song.melody[thisNote]));
    int divider      = (int16_t)pgm_read_word(&(song.melody[thisNote + 1]));

    int noteDuration;
    if (divider > 0) {
      noteDuration = wholenote / divider;
    } else if (divider < 0) {
      // Dotted notes: negative durations mean 1.5x the base length
      noteDuration = wholenote / abs(divider);
      noteDuration = (int)(noteDuration * 1.5);
    } else {
      noteDuration = wholenote / 4; // Fallback
    }

    // SAFETY CLAMP: NewTone only supports 31 Hz to 5000 Hz.
    // Anything outside this range (rests, corrupt reads) will be muted instead of screeching!
    Serial.println(currentPitch);
    if (currentPitch >= 31 && currentPitch <= 5000) {
      NewTone(buzzer, currentPitch, (int)(noteDuration * 0.9));
    } else {
      noNewTone(buzzer); // Mute for rests or glitch values
    }

    // Wait out the note duration while polling the joystick
    waitAndCheckJoystick(noteDuration);

    noNewTone(buzzer);

    registerMissedNotes();
    advanceGameClock();
    refreshDisplay();

    noNewTone(buzzer);
  }
}

void waitAndCheckJoystick(unsigned long duration) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    checkJoystickHit();
  }
}

// ---------------- Joystick input ----------------

// Which column the stick is currently pointing at. Absolute thirds of the
// full range, rather than relative-to-center, so no calibration step is
// needed -- the wide center zone naturally soaks up resting drift.
int getSelectedColumn() {
  int raw = analogRead(JOYSTICK_X_PIN);
  if (raw < 341) return 0;   // left
  if (raw > 682) return 2;   // right
  return 1;                  // center
}

// Debounced press detector: fires true exactly once per confirmed click.
bool joystickClickEdge() {
  int reading = digitalRead(JOYSTICK_SW_PIN);

  if (reading != swRawLast) {
    swDebounceStart = millis();
  }

  bool fired = false;
  if ((millis() - swDebounceStart) > debounceDelay) {
    if (reading != swConfirmed) {
      swConfirmed = reading;
      if (swConfirmed == LOW) {
        fired = true;
      }
    }
  }

  swRawLast = reading;
  return fired;
}

void checkJoystickHit() {
  if (joystickClickEdge()) {
    int col = getSelectedColumn();
    onNoteHitAttempt(col);
  }
  // int col = getSelectedColumn();
  // onNoteHitAttempt(col);
}

void onNoteHitAttempt(int col) {
  if (matrix[BOTTOM_ROW][col] == 1) {
    hits++;
    score += 100;
    matrix[BOTTOM_ROW][col] = 0; // clear it so it can't be double-scored
    refreshDisplay();            // immediate visual feedback
    updateLiveScore();
  } else {
    wrongPresses++;
  }
}

// ---------------- Game clock / grid ----------------

void registerMissedNotes() {
  for (int col = 0; col < NUM_COLS; col++) {
    if (matrix[BOTTOM_ROW][col] == 1) {
      misses++;
    }
  }
}

void advanceGameClock() {
  for (int col = 0; col < NUM_COLS; col++) {
    for (int row = NUM_ROWS - 1; row > 0; row--) {
      matrix[row][col] = matrix[row - 1][col];
    }
    matrix[0][col] = 0;
  }
  int spawnCol = random(NUM_COLS);
  matrix[0][spawnCol] = 1;
}

// SoftPWM handles the actual fade -- this just tells it the target level.
void refreshDisplay() {
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < NUM_COLS; col++) {
      SoftPWMSet(ledGrid[row][col], matrix[row][col] ? 255 : 0);
    }
  }
}

// ---------------- LCD text ----------------

void updateLiveScore() {
  lcd.setCursor(0, 1);
  lcd.print(F("                ")); // clear row 1
  lcd.setCursor(0, 1);
  lcd.print(F("Score: "));
  lcd.print(score);
}

void showEndScreens() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Song complete!"));
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Score: "));
  lcd.print(score);
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Hits:"));
  lcd.print(hits);
  lcd.print(F(" Miss:"));
  lcd.print(misses);
  lcd.setCursor(0, 1);
  lcd.print(F("Wrong presses:"));
  lcd.print(wrongPresses);
  delay(2000);
}
