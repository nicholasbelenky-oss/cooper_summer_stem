#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Library: "LiquidCrystal I2C" by Frank de Brabander (install via Library Manager).
// If lcd.init() doesn't compile on your version, replace it with lcd.begin(16, 2).

// If the screen stays blank, your module's address may be 0x3F instead of 0x27.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- Pins ----------------
const int VRX_PIN = A0;
const int VRY_PIN = A1;
const int SW_PIN  = 12;
const int five_volt = 13;

// ---------------- Display / world geometry ----------------
const int COLS = 16;
const int ROWS = 2;

// ---------------- Tunables ----------------
const int JOY_DEADZONE = 150;   // how far off-center before a direction registers
int joyCenterX = 512;           // overwritten by calibration in setup()
int joyCenterY = 512;

const bool DEBUG = true;        // set false once everything works, to quiet Serial

const unsigned long MOVE_INTERVAL           = 180;  // ms between player steps
const unsigned long PLAYER_BULLET_INTERVAL  = 90;   // ms between player bullet steps
const unsigned long ENEMY_BULLET_INTERVAL   = 140;  // ms between enemy bullet steps
const unsigned long ENEMY_THINK_INTERVAL    = 900;  // ms between enemy shots
const unsigned long ENEMY_SPAWN_INTERVAL    = 3000; // ms between enemy spawns
const unsigned long INVULNERABLE_MS         = 1000; // grace period after getting hit
const unsigned long DEBOUNCE_MS             = 25;

const int MAX_PLAYER_BULLETS = 3;
const int MAX_ENEMY_BULLETS  = 4;
const int MAX_ENEMIES        = 2;
const int STARTING_LIVES     = 3;

enum Dir { DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

struct Bullet {
  bool active;
  int col, row;
  Dir dir;
};

struct Enemy {
  bool alive;
  int col, row;
  unsigned long lastShot;
};

// ---------------- Game state ----------------
int playerCol, playerRow;
Dir facing = DIR_RIGHT;

Bullet playerBullets[MAX_PLAYER_BULLETS];
Bullet enemyBullets[MAX_ENEMY_BULLETS];
Enemy  enemies[MAX_ENEMIES];

int lives = STARTING_LIVES;
int score = 0;
bool gameOver = false;

bool invulnerable = false;
unsigned long invulnerableUntil = 0;

unsigned long lastMoveTime = 0;
unsigned long lastPlayerBulletTick = 0;
unsigned long lastEnemyBulletTick = 0;
unsigned long lastEnemySpawn = 0;

// Button debounce state (two-variable pattern: raw reading vs confirmed state)
int swRawLast = HIGH;
int swConfirmed = HIGH;
unsigned long swDebounceStart = 0;

char screenBuf[ROWS][COLS + 1];

// ---------------- Forward declarations ----------------
void startNewGame();
void waitForClick();
void showMessage(const char* line0, const char* line1);
bool buttonPressedEdge();
void readAndApplyMovement();
void handleShootInput();
void spawnPlayerBullet();
void updatePlayerBullets();
bool tryHitEnemyAt(int col, int row);
void updateEnemies();
void spawnEnemyBullet(Enemy &e);
void updateEnemyBullets();
void checkPlayerHit();
void flashHit();
void endGame();
void render();

void setup() {
  pinMode(SW_PIN, INPUT_PULLUP);
  if (DEBUG) Serial.begin(9600);
  pinMode(five_volt, OUTPUT);
  digitalWrite(five_volt, HIGH);
  lcd.init();
  lcd.backlight();
  randomSeed(analogRead(A2)); // floating/unused analog pin as a noise source

  calibrateJoystick();
  startNewGame();
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

  if (DEBUG) {
    Serial.print("Joystick calibrated. Center X=");
    Serial.print(joyCenterX);
    Serial.print(" Y=");
    Serial.println(joyCenterY);
  }
}

void loop() {
  if (gameOver) {
    if (buttonPressedEdge()) {
      startNewGame();
    }
    return;
  }

  readAndApplyMovement();
  handleShootInput();
  updatePlayerBullets();
  updateEnemyBullets();
  updateEnemies();
  checkPlayerHit();
  render();

  if (lives <= 0) {
    endGame();
  }
}

// ---------------- Setup / lifecycle ----------------

void startNewGame() {
  playerCol = COLS / 2;
  playerRow = 0;
  facing = DIR_RIGHT;
  lives = STARTING_LIVES;
  score = 0;
  gameOver = false;
  invulnerable = false;

  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) playerBullets[i].active = false;
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++)  enemyBullets[i].active = false;
  for (int i = 0; i < MAX_ENEMIES; i++)        enemies[i].alive = false;

  lastEnemySpawn = millis() - ENEMY_SPAWN_INTERVAL; // spawn one almost immediately

  showMessage("Bullet Hell!", "Click to start");
  waitForClick();
  lcd.clear();
}

void waitForClick() {
  while (digitalRead(SW_PIN) == HIGH) { }
  delay(DEBOUNCE_MS);
  while (digitalRead(SW_PIN) == LOW) { }
  delay(DEBOUNCE_MS);
}

void showMessage(const char* line0, const char* line1) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  lcd.print(line1);
}

// ---------------- Input ----------------

// Debounced press detector: fires true exactly once per confirmed press.
bool buttonPressedEdge() {
  int reading = digitalRead(SW_PIN);

  if (reading != swRawLast) {
    swDebounceStart = millis();
  }

  bool firedEdge = false;
  if ((millis() - swDebounceStart) > DEBOUNCE_MS) {
    if (reading != swConfirmed) {
      swConfirmed = reading;
      if (swConfirmed == LOW) {
        firedEdge = true;
      }
    }
  }

  swRawLast = reading;
  return firedEdge;
}

Dir readJoystickDirection() {
  int rx = analogRead(VRX_PIN);
  int ry = analogRead(VRY_PIN);

  int dx = rx - joyCenterX;
  int dy = ry - joyCenterY;

  // NOTE: if left/right or up/down feel swapped or inverted on your specific
  // joystick module, swap rx/ry above, or flip the > / < signs below.
  Dir desired = DIR_NONE;
  if (abs(dx) > abs(dy)) {
    if (dx > JOY_DEADZONE) desired = DIR_RIGHT;
    else if (dx < -JOY_DEADZONE) desired = DIR_LEFT;
  } else {
    if (dy > JOY_DEADZONE) desired = DIR_DOWN;
    else if (dy < -JOY_DEADZONE) desired = DIR_UP;
  }

  if (DEBUG) {
    Serial.print("rx="); Serial.print(rx);
    Serial.print(" ry="); Serial.print(ry);
    Serial.print(" dx="); Serial.print(dx);
    Serial.print(" dy="); Serial.print(dy);
    Serial.print(" dir="); Serial.println(desired);
  }

  return desired;
}

void readAndApplyMovement() {
  Dir desired = readJoystickDirection();

  // Update aim EVERY frame, independent of the movement cooldown below --
  // this is what lets you point the stick and fire without needing to
  // actually take a step first.
  if (desired != DIR_NONE) {
    facing = desired;
  }

  if (desired == DIR_NONE) return;
  if (millis() - lastMoveTime < MOVE_INTERVAL) return;

  lastMoveTime = millis();

  switch (desired) {
    case DIR_LEFT:  if (playerCol > 0) playerCol--; break;
    case DIR_RIGHT: if (playerCol < COLS - 1) playerCol++; break;
    case DIR_UP:    playerRow = 0; break;
    case DIR_DOWN:  playerRow = 1; break;
    default: break;
  }
}

void handleShootInput() {
  if (buttonPressedEdge()) {
    if (DEBUG) Serial.println("SW pressed -> firing");
    spawnPlayerBullet();
  }
}

void spawnPlayerBullet() {
  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    if (!playerBullets[i].active) {
      playerBullets[i].active = true;
      playerBullets[i].col = playerCol;
      playerBullets[i].row = playerRow;
      playerBullets[i].dir = facing;
      return;
    }
  }
  // all player bullet slots full -- shot is dropped
}

// ---------------- Bullets ----------------

void updatePlayerBullets() {
  if (millis() - lastPlayerBulletTick < PLAYER_BULLET_INTERVAL) return;
  lastPlayerBulletTick = millis();

  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    if (!playerBullets[i].active) continue;
    Bullet &b = playerBullets[i];

    if (b.dir == DIR_UP || b.dir == DIR_DOWN) {
      // Only 2 rows exist -- a vertical shot is a one-step strike into the
      // other row at the same column, then it's gone.
      int otherRow = 1 - b.row;
      if (tryHitEnemyAt(b.col, otherRow)) {
        score++;
      } else {
        tryHitEnemyBulletAt(b.col, otherRow); // shoot down an incoming bullet if one's there
      }
      b.active = false;
      continue;
    }

    b.col += (b.dir == DIR_RIGHT) ? 1 : -1;
    if (b.col < 0 || b.col >= COLS) {
      b.active = false;
      continue;
    }
    if (tryHitEnemyAt(b.col, b.row)) {
      score++;
      b.active = false;
    } else if (tryHitEnemyBulletAt(b.col, b.row)) {
      b.active = false;
    }
  }
}

bool tryHitEnemyAt(int col, int row) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].alive && enemies[i].col == col && enemies[i].row == row) {
      enemies[i].alive = false;
      return true;
    }
  }
  return false;
}

// Shoots down an incoming enemy bullet at this cell, if one's there.
bool tryHitEnemyBulletAt(int col, int row) {
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (enemyBullets[i].active && enemyBullets[i].col == col && enemyBullets[i].row == row) {
      enemyBullets[i].active = false;
      return true;
    }
  }
  return false;
}

// The reverse check: an enemy bullet stepping into a cell your bullet occupies.
bool tryDestroyPlayerBulletAt(int col, int row) {
  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    if (playerBullets[i].active && playerBullets[i].col == col && playerBullets[i].row == row) {
      playerBullets[i].active = false;
      return true;
    }
  }
  return false;
}

void updateEnemyBullets() {
  if (millis() - lastEnemyBulletTick < ENEMY_BULLET_INTERVAL) return;
  lastEnemyBulletTick = millis();

  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (!enemyBullets[i].active) continue;
    Bullet &b = enemyBullets[i];
    b.col += (b.dir == DIR_RIGHT) ? 1 : -1;
    if (b.col < 0 || b.col >= COLS) {
      b.active = false;
      continue;
    }
    // If it just stepped into a cell your bullet currently occupies, both die.
    if (tryDestroyPlayerBulletAt(b.col, b.row)) {
      b.active = false;
    }
  }
}

// ---------------- Enemies ----------------

void updateEnemies() {
  if (millis() - lastEnemySpawn > ENEMY_SPAWN_INTERVAL) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
      if (!enemies[i].alive) {
        enemies[i].alive = true;
        enemies[i].row = random(0, 2);
        // Spawn on whichever edge is farther from the player.
        enemies[i].col = (playerCol < COLS / 2) ? (COLS - 1) : 0;
        enemies[i].lastShot = millis();
        lastEnemySpawn = millis();
        break;
      }
    }
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive) continue;
    if (millis() - enemies[i].lastShot > ENEMY_THINK_INTERVAL) {
      enemies[i].lastShot = millis();
      spawnEnemyBullet(enemies[i]);
    }
  }
}

void spawnEnemyBullet(Enemy &e) {
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (!enemyBullets[i].active) {
      enemyBullets[i].active = true;
      enemyBullets[i].col = e.col;
      enemyBullets[i].row = e.row;
      // Bullets travel away from whichever edge the enemy is camped on.
      enemyBullets[i].dir = (e.col == 0) ? DIR_RIGHT : DIR_LEFT;
      return;
    }
  }
}

// ---------------- Player hit handling ----------------

void checkPlayerHit() {
  if (invulnerable) {
    if (millis() > invulnerableUntil) invulnerable = false;
    else return;
  }

  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (enemyBullets[i].active &&
        enemyBullets[i].col == playerCol &&
        enemyBullets[i].row == playerRow) {
      enemyBullets[i].active = false;
      lives--;
      invulnerable = true;
      invulnerableUntil = millis() + INVULNERABLE_MS;
      flashHit();
      return;
    }
  }
}

void flashHit() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hit! Lives: ");
  lcd.print(lives);
  delay(500); // brief pause so the player actually registers it
  lcd.clear();
}

void endGame() {
  gameOver = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
  delay(400); // avoid the death-causing click instantly restarting the game
}

// ---------------- Rendering ----------------

void render() {
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) screenBuf[r][c] = ' ';
    screenBuf[r][COLS] = '\0';
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].alive) screenBuf[enemies[i].row][enemies[i].col] = 'M';
  }
  for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (enemyBullets[i].active) screenBuf[enemyBullets[i].row][enemyBullets[i].col] = '*';
  }
  for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
    if (playerBullets[i].active) screenBuf[playerBullets[i].row][playerBullets[i].col] = '-';
  }
  // Player drawn last so it's always visible even if a bullet shares its cell.
  screenBuf[playerRow][playerCol] = '@';

  lcd.setCursor(0, 0);
  lcd.print(screenBuf[0]);
  lcd.setCursor(0, 1);
  lcd.print(screenBuf[1]);
}
