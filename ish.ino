/*
Ish Game
Requires LCD screen, potentiometer, LED (+ 220 Ohm Resistor), Passive Buzzer, Joystick

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

Joystick
GND - Negative
+5V - Positive
VRx - A4
VRy - A5
SW - 4

Connect positive side of LED to resistor, and that resistor to pin 6
Connect positive side of buzzer to pin 3
Start the game by clicking the joystick.
*/
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, A0, A1, A2, A3);

// Pin assignments
const int JOY_X = A4;
const int JOY_Y = A5;
const int BUTTON = 4; 
const int BUZZER = 3;
const int LED = 6;

int score = 0;
bool gameActive = false;
int failCount = 0;
int aiFailCount = 0;
bool usedDirections[4]; // 0=UP, 1=DOWN, 2=LEFT, 3=RIGHT
int currentDirection = -1;
bool awaitingInput = false;
bool playerAttacking = false;
int lastPlayerDir = -1;
int lastDefendDir = -1;

int difficulty = 0; // 0 = Easy, 1 = Medium, 2 = Hard
unsigned long inputStartTime = 0;
unsigned long reactionTime = 0;
int aiMissChance = 40; // default to Easy

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
  lcd.begin(16, 2);
  randomSeed(analogRead(A2));
  showMenu();
}

void showMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select difficulty");
  lcd.setCursor(0, 1);
  lcd.print("0-E 1-M 2-H");
  while (true) {
    int sel = analogRead(JOY_X);
    if (sel <= 100) {
      difficulty = 0;
      aiMissChance = 40;
      Serial.println("Difficulty set to EASY");
      break;
    } else if (sel >= 1000) {
      difficulty = 2;
      aiMissChance = 0;
      Serial.println("Difficulty set to HARD");
      break;
    } else if (analogRead(JOY_Y) >= 1000) {
      difficulty = 1;
      aiMissChance = 40;
      Serial.println("Difficulty set to MEDIUM");
      break;
    }
    delay(100);
  }
  lcd.clear();
  lcd.print("Press to start!");
}

void startRound() {
  score = 0;
  failCount = 0;
  aiFailCount = 0;
  lastPlayerDir = -1;
  lastDefendDir = -1;
  resetDirections();
  lcd.clear();
  lcd.print("Round start!");
  Serial.println("Round started");
  delay(1000);
  nextIsh();
  gameActive = true;
  playerAttacking = false;
}

void countdownBeep() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER, 1000, 100);
    delay(300);
  }
}

void nextIsh() {
  lcd.clear();
  currentDirection = pickUnusedDirection();
  usedDirections[currentDirection] = true;
  countdownBeep();
  lcd.setCursor(0, 0);
  lcd.print("ISH → ");
  lcd.print(getDirName(currentDirection));
  Serial.print("New AI attack direction: ");
  Serial.println(getDirName(currentDirection));
  digitalWrite(LED, HIGH);
  awaitingInput = true;
  playerAttacking = false;
  inputStartTime = millis();
  reactionTime = (difficulty == 1) ? 500 : (difficulty == 2) ? 400 : 0;
}

void quickNextIsh() {
  lcd.clear();
  currentDirection = pickUnusedDirection();
  usedDirections[currentDirection] = true;
  lcd.setCursor(0, 0);
  lcd.print("ISH → ");
  lcd.print(getDirName(currentDirection));
  Serial.print("Quick AI attack direction: ");
  Serial.println(getDirName(currentDirection));
  digitalWrite(LED, HIGH);
  awaitingInput = true;
  playerAttacking = false;
  inputStartTime = millis();
  reactionTime = (difficulty == 1) ? 500 : (difficulty == 2) ? 400 : 0;
}

int getDirection(int x, int y) {
  if (x >= 1000) return 3;
  if (x <= 100) return 2;
  if (y >= 1000) return 1;
  if (y <= 100) return 0;
  return -1;
}

String getDirName(int dir) {
  switch (dir) {
    case 0: return "UP";
    case 1: return "DOWN";
    case 2: return "LEFT";
    case 3: return "RIGHT";
    default: return "?";
  }
}

int pickUnusedDirection() {
  while (true) {
    int d = random(0, 4);
    if (!usedDirections[d]) return d;
  }
}

void resetDirections() {
  for (int i = 0; i < 4; i++) usedDirections[i] = false;
  Serial.println("Direction pool reset");
}

void loop() {
  if (!gameActive) {
    if (digitalRead(BUTTON) == LOW) {
      delay(300);
      startRound();
    }
    return;
  }

  if (!awaitingInput) return;

  if (!playerAttacking && reactionTime > 0 && millis() - inputStartTime > reactionTime) {
    tone(BUZZER, 400, 200);
    gameActive = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You lose!");
    lcd.setCursor(0, 1);
    lcd.print("React Faster!");
    Serial.println("Player failed to react in time");
    tone(BUZZER, 200, 500);
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    return;
  }

  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);
  int playerDir = getDirection(x, y);

  if (playerDir != -1 && awaitingInput) {
    Serial.print("Player input: ");
    Serial.println(getDirName(playerDir));
    digitalWrite(LED, LOW);
    awaitingInput = false;

    if (playerAttacking) {
      lastPlayerDir = playerDir;
      delay(100);
      int aiDefend;
      bool aiTakesHit = (random(100) < aiMissChance);
      if (aiTakesHit) {
        aiDefend = playerDir;
        Serial.println("AI takes hit on purpose");
      } else {
        while(true) {
          aiDefend = random(0, 4);
          Serial.print("Rolled a ");
          Serial.println(aiDefend);
          Serial.print("Was this a used direction? ");
          Serial.println(usedDirections[aiDefend]);
          if(!usedDirections[aiDefend]) {
            break;
            }
          }
          Serial.print("AI dodges with direction: ");
          Serial.println(getDirName(aiDefend));
      }
      Serial.print("Adding ");
      Serial.print(aiDefend);
      Serial.println(" to the used directions.");
      usedDirections[aiDefend] = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AI Defends: ");
      lcd.print(getDirName(aiDefend));

      if (aiDefend == playerDir) {
        lcd.setCursor(0, 1);
        lcd.print("Hit!");
        tone(BUZZER, 400, 200);
        aiFailCount++;
        Serial.println("AI was hit");
        if (aiFailCount >= 4) {
          gameActive = false;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("KO! You Win");
          tone(BUZZER, 1000, 500);
          digitalWrite(LED, HIGH);
          delay(1000);
          digitalWrite(LED, LOW);
          lcd.setCursor(0, 1);
          lcd.print("Score: ");
          lcd.print(score);
          return;
        }
        delay(400);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Your turn!");
        lcd.setCursor(0, 1);
        lcd.print("Hit me!");
        digitalWrite(LED, HIGH);
        awaitingInput = true;
        inputStartTime = millis();
        playerAttacking = true;
      } else {
        lcd.setCursor(0, 1);
        lcd.print("Missed!");
        tone(BUZZER, 880, 200);
        aiFailCount = 0;
        resetDirections();
        delay(1000);
        nextIsh();
      }
    } else {
      
      if (playerDir == currentDirection) {
        tone(BUZZER, 400, 200);
        usedDirections[currentDirection] = true;
        failCount++;
        lastDefendDir = playerDir;
        Serial.println("Player was hit");
        if (failCount >= 4) {
          gameActive = false;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("KO! You Lose");
          tone(BUZZER, 200, 500);
          digitalWrite(LED, HIGH);
          delay(1000);
          digitalWrite(LED, LOW);
          lcd.setCursor(0, 1);
          lcd.print("Score: ");
          lcd.print(score);
          return;
        }
        delay(400);
        quickNextIsh();
      } else {
        if (playerDir == lastDefendDir) {
          gameActive = false;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("You lose! Repeat");
          lcd.setCursor(0, 1);
          lcd.print("direction!");
          Serial.println("Player repeated direction");
          tone(BUZZER, 200, 500);
          digitalWrite(LED, HIGH);
          delay(1000);
          digitalWrite(LED, LOW);
          return;
        }
        lastDefendDir = -1;
        lcd.clear();
        lcd.print("Survived!");
        Serial.println("Player survived");
        resetDirections();
        failCount = 0;
        score++;
        tone(BUZZER, 880, 200);
        delay(300);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Your turn!");
        lcd.setCursor(0, 1);
        lcd.print("Hit me!");

        playerAttacking = true;
        digitalWrite(LED, HIGH);
        awaitingInput = true;
        inputStartTime = millis();
      }
    }
  }
}
