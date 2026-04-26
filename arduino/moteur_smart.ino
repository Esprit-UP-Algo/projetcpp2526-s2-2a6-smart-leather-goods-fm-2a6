#include <Servo.h>

Servo servo;
#define PIN_SERVO 9

void setup() {
  Serial.begin(9600);
  servo.attach(PIN_SERVO);
  servo.write(90); // position neutre
  delay(500);
  Serial.println("READY");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "MOTOR:1") {
      // 1er choix = 1/2 cercle
      servo.write(0);
      delay(500);
      servo.write(180);
      delay(1000);
      servo.write(90);
      Serial.println("DONE:1");

    } else if (cmd == "MOTOR:2") {
      // 2ème choix = 1/4 cercle
      servo.write(90);
      delay(300);
      servo.write(180);
      delay(800);
      servo.write(90);
      Serial.println("DONE:2");

    } else if (cmd == "PING") {
      Serial.println("PONG");
    }
  }
}
