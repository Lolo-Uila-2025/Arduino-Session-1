/*
A sensor that sends a signal to pin 6 (An LED) when it detects an object within 50cm.
Requires an Ultrasonic Sensor and LED (+ 220 ohm resistor).

How to wire:
Ultrasonic Sensor
Vcc - Positive
Trig - 12
Echo - 11
Gnd - Negative

Connect positive side of LED to resistor, and that resistor to pin 6
*/
#include "SR04.h"

#define TRIG_PIN 12
#define ECHO_PIN 11
#define LED_PIN 6 

SR04 sr04 = SR04(ECHO_PIN, TRIG_PIN);
long distance;

void setup() {
   Serial.begin(9600);
   pinMode(LED_PIN, OUTPUT);  // Set LED pin as output
   delay(1000);
}

void loop() {
   distance = sr04.Distance();
   Serial.print(distance);
   Serial.println("cm");

   if (distance > 0 && distance <= 50) {
      digitalWrite(LED_PIN, HIGH);  // Turn on LED
   } else {
      digitalWrite(LED_PIN, LOW);   // Turn off LED
   }

   delay(100);
}
