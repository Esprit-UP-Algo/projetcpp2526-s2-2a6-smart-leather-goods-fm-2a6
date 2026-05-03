#include <Servo.h>

Servo servo;
const int PIN_SERVO = 9;
const int ANGLE_1 = 180;
const int ANGLE_2 = 90;
const int STEP_DELAY_MS = 15;
const int HOLD_MS = 800;
const int LOCK_MS = 2000;
const int WATCHDOG_MS = 5000;

int currentAngle = 0;
unsigned long lockUntil = 0;
unsigned long lastCommandTime = 0;
bool commandReceived = false;

void moveSmoothTo(int target) {
  if (target > currentAngle) {
    for (int a = currentAngle; a <= target; a++) {
      servo.write(a);
      delay(STEP_DELAY_MS);
    }
  } else {
    for (int a = currentAngle; a >= target; a--) {
      servo.write(a);
      delay(STEP_DELAY_MS);
    }
  }
  currentAngle = target;
}

void runMove(int target, const char* movingMsg, const char* doneMsg) {
  if (commandReceived == true) {
  servo.attach(PIN_SERVO);
  delay(80);

  Serial.println(movingMsg);
  moveSmoothTo(target);
  delay(HOLD_MS);
  moveSmoothTo(0);
  servo.write(0);
  delay(500);

  servo.detach();
  pinMode(PIN_SERVO, INPUT);

  lockUntil = millis() + LOCK_MS;
  lastCommandTime = millis();
  Serial.println(doneMsg);
  }
}

void setup() {
  Serial.begin(9600);

  servo.attach(PIN_SERVO);
  currentAngle = 0;
  servo.write(0);
  delay(500);
  servo.detach();
  pinMode(PIN_SERVO, INPUT);

  // READY une seule fois au démarrage.
  Serial.println("READY");
}

void loop() {
  // Watchdog: garder le servo détaché après inactivité.
  if (millis() - lastCommandTime > WATCHDOG_MS) {
    if (servo.attached()) {
      servo.detach();
      pinMode(PIN_SERVO, INPUT);
    }
  }

  // 1) Vérifier d'abord la présence de commande série.
  if (!Serial.available()) return;

  // 2) Verrou anti-doublon pendant 2s après un mouvement.
  if (millis() < lockUntil) {
    while (Serial.available()) Serial.read();
    return;
  }

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  while (Serial.available()) Serial.read();

  // 3) Mouvement uniquement pour commandes valides.
  if (cmd == "1" || cmd == "MOTOR:1") {
    commandReceived = true;
    runMove(ANGLE_1, "MOVING:1", "DONE:1");
  } else if (cmd == "2" || cmd == "MOTOR:2") {
    commandReceived = true;
    runMove(ANGLE_2, "MOVING:2", "DONE:2");
  } else {
    commandReceived = false;
  }
  // Tout autre message est ignoré.
}
