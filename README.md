# 💧 IV Bag Monitoring System

An embedded health monitoring solution that tracks the remaining fluid in an IV (Intravenous) bag using a weight sensor. The system alerts medical staff via LED and buzzer when fluid levels reach warning or critical thresholds, ensuring timely IV replacement and enhanced patient safety.

---

## 🚀 Overview

This project uses an **Arduino UNO**, **HX711 Load Cell module**, and optional **LCD display** to measure the weight of an IV bag and determine how much fluid remains. When the fluid falls below preset thresholds, alerts are triggered to notify the caregiver.

---

## 🛠️ Hardware Components

| Component         | Description                       |
|------------------|-----------------------------------|
| Arduino UNO       | Main microcontroller               |
| HX711 Module      | Amplifier for the load cell        |
| Load Cell         | Measures the IV bag weight         |
| 16x2 LCD (Optional) | Displays fluid weight and status |
| Buzzer            | Alerts for low/critical level      |
| LED               | Visual indicator of warning status |
| Power Source      | USB or battery                     |

---

## ⚙️ How It Works

1. The **load cell** senses the weight of the IV bag.
2. The **HX711** amplifies and sends the data to the Arduino.
3. Arduino calculates the fluid level based on weight.
4. When fluid is **low (<150g)** or **critical (<50g)**:
   - **LED** lights up.
   - **Buzzer** sounds for critical level.
   - **LCD** (if used) shows real-time status.
5. When a new IV bag is added, the system resets automatically.

---

## 🧑‍💻 Arduino Code

Below is the main sketch that powers the monitoring system:

```cpp
#include "HX711.h"
#include <LiquidCrystal.h>

// Pin Definitions
#define DT A1
#define SCK A0
#define BUZZER_PIN 9
#define LED_PIN 8

// LCD (optional)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// HX711
HX711 scale;
float calibration_factor = -7050; // Adjust based on your load cell

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("IV Bag Monitor");

  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare(); // Reset the scale to 0

  delay(2000);
  lcd.clear();
}

void loop() {
  float weight = scale.get_units();
  Serial.print("Weight (g): ");
  Serial.println(weight);

  lcd.setCursor(0, 0);
  lcd.print("Weight: ");
  lcd.print(weight, 1);
  lcd.print("g   ");

  if (weight < 50) {  // Critical threshold
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("CRITICAL! REFILL ");
  } else if (weight < 150) {  // Warning threshold
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Warning: Low    ");
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Status: Normal  ");
  }

  delay(1000);
}
