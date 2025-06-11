/*
Reaction Test Game
Requires LCD screen, potentiometer, two buttons, and an LED (+ 220 ohm resistor)

How to Wire:
LCD Screen
VSS - Negative
VDD - Positive
V0 - Top of potentiometer
RS - 7
RW - Negative
E - 8
D4 - A0
D5 - A1
D6 - A2
D7 - A3
A - Positive
K - Negative

Connect positive side of LED to resistor, and that resistor to pin 6
Connect left side of button #1 to 5, right side to negative
Connect left side of button #2 to 4, right side to negative

*/

#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, A0, A1, A2, A3);
int button1 = 5;
int button2 = 4;
int led = 6;
int state = 0;
int timer = 0;
int startTime = 0;
int score;

void setup() {
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Press 1 to start!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
if (state == 0) {
  if(digitalRead(button1) == LOW) {
    lcd.setCursor(0, 0);
    lcd.print("Ready?...       ");
    delay(100);
    state = 1;
  }
}

if (state == 1) {
  timer = random(1000, 10000);
  delay(timer);
  digitalWrite(led, HIGH);
  startTime = millis();
  state = 2;
}

if (state == 2) {
  if (digitalRead(button1) == LOW) {
    score = millis() - startTime;
    digitalWrite(led, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Your score is...   ");
    lcd.setCursor(0, 1);
    lcd.print(score);
    lcd.print(" miliseconds!");
    state = 3;
  }
}

if (digitalRead(button2) == LOW) {
  state = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press 1 to start!");
}

}
