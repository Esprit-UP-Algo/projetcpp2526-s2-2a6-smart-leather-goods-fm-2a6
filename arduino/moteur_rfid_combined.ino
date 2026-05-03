#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

Servo servo;
#define PIN_SERVO 9
#define SS_PIN 10
#define RST_PIN 8

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  servo.attach(PIN_SERVO);
  servo.write(0);
  delay(500);
  Serial.println("READY");
}

void loop() {
  // Gérer commandes série (moteur)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "MOTOR:1") {
      servo.write(0);   delay(400);
      servo.write(180); delay(600);
      servo.write(0);   delay(400);
      while (Serial.available()) Serial.read();
      Serial.println("DONE:1");

    } else if (cmd == "MOTOR:2") {
      servo.write(0);  delay(400);
      servo.write(90); delay(600);
      servo.write(0);  delay(400);
      while (Serial.available()) Serial.read();
      Serial.println("DONE:2");

    } else if (cmd == "PING") {
      Serial.println("PONG");
    }
  }

  // Gérer RFID (non bloquant)
  if (rfid.PICC_IsNewCardPresent() &&
      rfid.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) uid += ":";
    }
    Serial.println("#RFID:UID:" + uid + "#");
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}
