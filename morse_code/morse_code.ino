#include "MorseEncoder.h"
MorseEncoder morseL(13);

void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600); 
  Serial.println("Terminal Ready. Type something and press Enter:");
  morseL.beginLight(15);
}

void loop() {
  // Check if there are characters available to read
  if (Serial.available() > 0) {
    // Read the incoming data until a newline character is received
    String userInput = Serial.readStringUntil('\n');
    
    // Trim any accidental whitespace or carriage returns
    userInput.trim(); 
    
    // Print a response back to the terminal
    Serial.print("Arduino received: ");
    Serial.println(userInput);
    morseL.print(userInput);
  }
}
