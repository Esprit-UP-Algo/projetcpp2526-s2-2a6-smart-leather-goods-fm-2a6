/*
 * ========================================
 * FIL D'OR - Module Servo Standalone
 * ========================================
 *
 * Composant : Servo moteur (SG90 ou similaire)
 * Port      : COM6
 * Baud      : 9600
 *
 * Cablage:
 *   VCC    (Rouge)              -> 5V
 *   GND    (Marron / Noir)      -> GND
 *   Signal (Orange/Jaune/Blanc) -> Pin 3
 *
 * Commandes :
 *   MOTOR:1        -> 1er choix  : position 90°
 *   MOTOR:2        -> 2e/3e choix: position 180°
 *   PING           -> PONG (detection automatique Qt)
 */

#include <Servo.h>

// ========== CONFIGURATION ==========
const int SERVO_PIN     = 3;
const int STEP_DELAY_MS = 15;   // ms entre chaque degre (mouvement fluide)
const int HOLD_MS       = 300;  // pause avant detach

// ========== ETAT ==========
Servo monServo;
int   currentAngle = 0;
String cmdBuffer   = "";

// ========== SETUP ==========
void setup() {
    Serial.begin(9600);
    monServo.attach(SERVO_PIN);
    monServo.write(0);
    delay(500);
    monServo.detach();
    Serial.println(F("READY"));
}

// ========== LOOP ==========
void loop() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            cmdBuffer.trim();
            if (cmdBuffer.length() > 0) {
                traiterCommande(cmdBuffer);
                cmdBuffer = "";
            }
        } else {
            cmdBuffer += c;
            if (cmdBuffer.length() > 32) cmdBuffer = "";
        }
    }
}

// ========== TRAITEMENT ==========
void traiterCommande(const String &cmd) {

    if (cmd == "PING") {
        Serial.println(F("PONG"));
        return;
    }

    // 1er choix -> 90 deg
    if (cmd == "1" || cmd == "MOTOR:1") {
        Serial.println(F("MOVING:1"));
        moveSmoothTo(90);
        delay(HOLD_MS);
        monServo.detach();
        Serial.println(F("DONE:1"));
        return;
    }

    // 2e et 3e choix -> 180 deg
    if (cmd == "2" || cmd == "MOTOR:2") {
        Serial.println(F("MOVING:2"));
        moveSmoothTo(180);
        delay(HOLD_MS);
        monServo.detach();
        Serial.println(F("DONE:2"));
        return;
    }
}

// ========== MOUVEMENT FLUIDE ==========
void moveSmoothTo(int target) {
    if (!monServo.attached()) {
        monServo.attach(SERVO_PIN);
        delay(20);
    }
    target = constrain(target, 0, 180);
    if (target > currentAngle) {
        for (int a = currentAngle; a <= target; a++) {
            monServo.write(a);
            delay(STEP_DELAY_MS);
        }
    } else {
        for (int a = currentAngle; a >= target; a--) {
            monServo.write(a);
            delay(STEP_DELAY_MS);
        }
    }
    currentAngle = target;
}
