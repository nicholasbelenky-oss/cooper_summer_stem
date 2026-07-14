//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

#define x_joystick A0
#define five_volt 13

uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
uint8_t note[8]  = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
uint8_t clock[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
uint8_t retarrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};
int center = 512;
int DEADZONE = 20;
const int max_length = 50;
static char message[max_length];
int dx = 0;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Serial.begin(9600);
  // lcd.createChar(0, bell);
  // lcd.createChar(1, note);
  // lcd.createChar(2, clock);
  // lcd.createChar(3, heart);
  // lcd.createChar(4, duck);
  // lcd.createChar(5, check);
  // lcd.createChar(6, cross);
  // lcd.createChar(7, retarrow);
  // lcd.home();
  
  // lcd.print("Hello world...");
  // lcd.setCursor(0, 1);
  // lcd.print(" i ");
  // lcd.printByte(3);
  // lcd.print(" arduinos!");
  // delay(5000);
  // displayKeyCodes();
  pinMode(five_volt, OUTPUT);
  digitalWrite(five_volt, HIGH);

}

void getMessage(int x) {
  static int position = 0;
  if (Serial.available() > 0) {
    lcd.clear();
    char incomingChar = Serial.read(); 
    if (incomingChar != '\n' && position < (max_length - 1)) {
      message[position] = incomingChar;
      position++;
    } 
    else {
      message[position] = '\0'; 
      lcd.setCursor(x,0);
      lcd.print(message);
      position = 0; 
    }
  }
}

// display all keycodes
void displayKeyCodes(void) {
  uint8_t i = 0;
  while (1) {
    lcd.clear();
    lcd.print("Codes 0x"); lcd.print(i, HEX);
    lcd.print("-0x"); lcd.print(i+16, HEX);
    lcd.setCursor(0, 1);
    for (int j=0; j<16; j++) {
      lcd.printByte(i+j);
    }
    i+=16;
    
    delay(4000);
  }
}

void loop() {
  dx = analogRead(x_joystick) - center;
  static int x = 8;
  getMessage(x);
  if (abs(dx) < DEADZONE) {
    
  }
  else if (dx < 0) {
    delay(100);
    x--;
    if (x>=0) {
      lcd.clear();
      lcd.setCursor(x, 0);
      lcd.print(message);
    }
    else {
      lcd.clear();
      lcd.print(&message[abs(x)]);
    }
  }
  else if (dx > 0) {
    delay(100);
    x++;
    if (x>=0) {
      lcd.clear();
      lcd.setCursor(x, 0);
      lcd.print(message);
    }
    else {
      lcd.clear();
      lcd.print(&message[abs(x)]);
    }
  }
  
}
