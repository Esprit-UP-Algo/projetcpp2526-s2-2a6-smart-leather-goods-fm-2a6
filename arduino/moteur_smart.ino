/*
 * MOTOR:1 : 0° -> 45° -> 90° -> 0°  (~2s)
 * MOTOR:2 : 0° -> 180° -> 0°        (~1.5s)
 */
#include <Servo.h>

Servo servo;
const int PIN_SERVO = 9;
const int STEP_DEG = 3;
const int STEP_MS = 2;
const int HOLD_MS = 60;
const int LOCK_MS = 120;
const int SETTLE_MS = 30;

bool attached = false;
bool moving = false;
int angle = 0;
unsigned long lockUntil = 0;

static void attachServo() {
  if (!attached) {
    servo.attach(PIN_SERVO);
    attached = true;
    delay(SETTLE_MS);
  }
}

static void detachServo() {
  if (attached) {
    servo.detach();
    attached = false;
    pinMode(PIN_SERVO, INPUT);
  }
}

static void goTo(int target) {
  attachServo();
  if (target > angle) {
    for (int a = angle; a < target; a += STEP_DEG) {
      servo.write(a);
      delay(STEP_MS);
    }
  } else {
    for (int a = angle; a > target; a -= STEP_DEG) {
      servo.write(a);
      delay(STEP_MS);
    }
  }
  servo.write(target);
  angle = target;
}

static void runChoice1() {
  moving = true;
  Serial.println("MOVING:1");
  Serial.flush();
  goTo(45);
  delay(HOLD_MS);
  goTo(90);
  delay(HOLD_MS);
  goTo(0);
  detachServo();
  Serial.println("DONE:1");
  Serial.flush();
  lockUntil = millis() + LOCK_MS;
  moving = false;
}

static void runChoice2() {
  moving = true;
  Serial.println("MOVING:2");
  Serial.flush();
  goTo(180);
  delay(HOLD_MS);
  goTo(0);
  detachServo();
  Serial.println("DONE:2");
  Serial.flush();
  lockUntil = millis() + LOCK_MS;
  moving = false;
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(40);
  angle = 0;
  attachServo();
  servo.write(0);
  delay(100);
  detachServo();
  Serial.println("READY");
  Serial.flush();
}

void loop() {
  if (moving)
    return;
  if (millis() < lockUntil)
    return;
  if (!Serial.available())
    return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0 || cmd.startsWith("#"))
    return;

  if (cmd == "1" || cmd == "MOTOR:1")
    runChoice1();
  else if (cmd == "2" || cmd == "MOTOR:2")
    runChoice2();
  else if (cmd == "PING") {
    Serial.println("PONG");
    Serial.flush();
  }
}
