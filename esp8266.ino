#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

// Pin definitions
#define DOUT   D5   // HX711 DT
#define CLK    D6   // HX711 SCK
#define BUZZER D7   // Buzzer

HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD 16x2 with I2C address 0x27

// Blynk credentials
char auth[] = "epRAk3Wa8BdtHCZJM_eR--88IMzd9r3R";
char ssid[] = "123456789";
char pass[] = "123456789";

// Global variables
float weight;
int liter;
int val;
unsigned long previousMillis = 0;
const long interval = 500;

// Calibration factor
float calibration_factor = -250000.0;

void connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");

    WiFi.begin(ssid, pass);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(500);
      lcd.print(".");
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Connected");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      delay(1500);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Failed");
      lcd.setCursor(0, 1);
      lcd.print("Retrying...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  lcd.begin();
  lcd.backlight();
  delay(500);  // Small delay before displaying

  lcd.setCursor(0, 0);
  lcd.print("IV Monitor Start");
  delay(1500);
  lcd.clear();

  connectToWiFi();
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare();
  Serial.print("Zero factor: ");
  Serial.println(scale.read_average());
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  Blynk.run();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    measureWeight();
  }
}

void measureWeight() {
  scale.set_scale(calibration_factor);
  weight = scale.get_units(5);
  if (weight < 0) weight = 0;

  liter = weight * 1000;
  liter = max(liter, 0);

  val = map(liter, 0, 1000, 0, 100);
  val = constrain(val, 0, 100);

  static int lastLiter = -1;
  static int lastVal = -1;

  if (liter != lastLiter || val != lastVal) {
    lastLiter = liter;
    lastVal = val;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BATTLE: ");
    lcd.print(liter);
    lcd.print("ml");

    lcd.setCursor(0, 1);
    lcd.print("Level: ");
    lcd.print(val);
    lcd.print("%");

    Serial.print("Weight: ");
    Serial.print(weight);
    Serial.println(" Kg");

    Serial.print("IV Bottle: ");
    Serial.print(liter);
    Serial.println(" mL");

    Serial.print("IV Percent: ");
    Serial.print(val);
    Serial.println("%");

    // Alert conditions
    if (val <= 50 && val > 20) {
      Blynk.logEvent("iv_alert_50", "IV Bottle is at 50%");
      tone(BUZZER, 1000, 100);
    } else if (val <= 20) {
      Blynk.logEvent("iv_alert_20", "IV Bottle is critically LOW!");
      tone(BUZZER, 2000, 300);
    } else {
      noTone(BUZZER);
    }
  }

  // Send data to Blynk App
  Blynk.virtualWrite(V0, liter);
  Blynk.virtualWrite(V1, val);
  Blynk.virtualWrite(V2, weight);
}
