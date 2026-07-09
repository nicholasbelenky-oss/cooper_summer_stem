// #include "merrychristmas.ino"
//------------------------------------

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0


// change this to whichever pin you want to use
int buzzer = A5;
// change this to make the song slower or faster
int tempo = 160;


// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {

  // We Wish You a Merry Christmas
  // Score available at https://musescore.com/user/6208766/scores/1497501

  NOTE_C5,4, //1
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,
  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8,
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,

  NOTE_F5,2, NOTE_C5,4, //8
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,
  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8,
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4,

  NOTE_F5,4, NOTE_F5,4, NOTE_F5,4,//17
  NOTE_E5,2, NOTE_E5,4,
  NOTE_F5,4, NOTE_E5,4, NOTE_D5,4,
  NOTE_C5,2, NOTE_A5,4,
  NOTE_AS5,4, NOTE_A5,4, NOTE_G5,4,
  NOTE_C6,4, NOTE_C5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4,
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,

  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8, //27
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,
  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8,
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4,
  NOTE_F5,4, NOTE_F5,4, NOTE_F5,4,
  NOTE_E5,2, NOTE_E5,4,
  NOTE_F5,4, NOTE_E5,4, NOTE_D5,4,

  NOTE_C5,2, NOTE_A5,4,//36
  NOTE_AS5,4, NOTE_A5,4, NOTE_G5,4,
  NOTE_C6,4, NOTE_C5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4,
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,

  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8,//45
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, NOTE_C5,4,
  NOTE_F5,4, NOTE_F5,8, NOTE_G5,8, NOTE_F5,8, NOTE_E5,8,
  NOTE_D5,4, NOTE_D5,4, NOTE_D5,4,
  NOTE_G5,4, NOTE_G5,8, NOTE_A5,8, NOTE_G5,8, NOTE_F5,8,
  NOTE_E5,4, NOTE_C5,4, NOTE_C5,4,

  NOTE_A5,4, NOTE_A5,8, NOTE_AS5,8, NOTE_A5,8, NOTE_G5,8, //53
  NOTE_F5,4, NOTE_D5,4, NOTE_C5,8, NOTE_C5,8,
  NOTE_D5,4, NOTE_G5,4, NOTE_E5,4,
  NOTE_F5,2, REST,4
};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;


// 3x3 Pin Layout Grid mapping directly to your hardware
const int ledGrid[3][3] = {
  {7, 10, 13},  // Top Row (Left, Center, Right)
  {6,  9, 12},  // Middle Row (Left, Center, Right)
  {5,  8, 11}   // Bottom Row (Left, Center, Right)
};

// Buttons: index 0 = Left column, 1 = Center column, 2 = Right column
const int buttonList[] = {2, 3, 4};

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
  Serial.begin(9600);

  // Set all 9 mapped pins as OUTPUT and turn them off initially
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      pinMode(ledGrid[row][col], OUTPUT);
      digitalWrite(ledGrid[row][col], LOW);
    }
  }

  // Buttons wired to GND, using internal pullups
  for (int i = 0; i < 3; i++) {
    pinMode(buttonList[i], INPUT_PULLUP);
  }

  playSong();

  // Song's over - print final score
  Serial.println("=== Song complete ===");
  Serial.print("Score: "); Serial.println(score);
  Serial.print("Hits: "); Serial.println(hits);
  Serial.print("Misses: "); Serial.println(misses);
  Serial.print("Wrong presses: "); Serial.println(wrongPresses);
}

void playSong() {
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration * 0.9);

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
  int currentNoteColumn = noteSequence[sequenceIndex];
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
  for (int row = 0; row < 3; row++) {
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
  // Game runs entirely inside playSong(), called once from setup().
}
