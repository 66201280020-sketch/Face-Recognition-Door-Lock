#include <Arduino.h>
#include <Wire.h>
#include <I2CKeyPad.h>

// ===== PIN CONFIG =====
#define RELAY_PIN        10    // รีเลย์ Active LOW
#define FACE_SIGNAL_PIN  7     // รับสัญญาณจาก ESP32-CAM

// ===== KEYPAD CONFIG =====
#define KEYPAD_ADDR 0x20       // HW-171 / PCF8574
I2CKeyPad keyPad(KEYPAD_ADDR);

// ===== PASSWORD =====
String inputPassword = "";
const String correctPassword = "1234";

// ===== FUNCTION =====
void openDoor() {
  Serial.println("\n[SYSTEM] >>> UNLOCKING DOOR <<<");
  digitalWrite(RELAY_PIN, LOW);    // เปิดรีเลย์
  delay(3000);                     // เปิด 3 วิ
  digitalWrite(RELAY_PIN, HIGH);   // ปิดรีเลย์
  inputPassword = "";
  Serial.println("[SYSTEM] DOOR LOCKED");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ----- RELAY -----
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);   // ปิดรีเลย์ไว้ก่อน

  // ----- FACE SIGNAL -----
  pinMode(FACE_SIGNAL_PIN, INPUT_PULLDOWN);

  // ----- I2C -----
  Wire.begin(8, 9); // SDA=8, SCL=9 (ESP32-S3)
  if (!keyPad.begin()) {
    Serial.println("❌ Keypad Not Found!");
  } else {
    Serial.println("✅ Keypad Ready");
  }

  Serial.println("ESP32-S3 Door Lock Ready");
  Serial.println("Waiting for Face or Keypad...");
}

void loop() {

  // ===== รับสัญญาณจาก ESP32-CAM =====
  if (digitalRead(FACE_SIGNAL_PIN) == HIGH) {
    Serial.println("[WIRE] Face Signal Detected");
    openDoor();

    // รอจนกว่าสัญญาณจะหาย (กันเด้ง)
    while (digitalRead(FACE_SIGNAL_PIN) == HIGH) {
      delay(10);
    }
  }

  // ===== อ่าน Keypad =====
  if (keyPad.isPressed()) {
    uint8_t index = keyPad.getKey();
    char keyMap[] = "123A456B789C*0#D";
    char key = keyMap[index];

    if (key >= '0' && key <= '9') {
      inputPassword += key;
      Serial.print("*");
    }
    else if (key == '#') {
      if (inputPassword == correctPassword) {
        openDoor();
      } else {
        Serial.println("\n[ERROR] Wrong Code!");
        inputPassword = "";
      }
    }
    else if (key == '*') {
      inputPassword = "";
      Serial.println("\n[INFO] Cleared");
    }

    while (keyPad.isPressed()) delay(10);
  }
}
