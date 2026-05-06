/*
 * Gas Sensor — COM5
 * Sends analog reading from A0 to Qt app via Serial at 9600 baud.
 * Adaptive delay: faster when gas level is high for responsive display.
 */

void setup() {
    Serial.begin(9600);
}

void loop() {
    unsigned int val = analogRead(A0);
    Serial.println(val);

    // Adaptive polling: urgent alerts get faster updates
    if      (val > 700) delay(200);   // DANGER  : 5 readings/s
    else if (val > 500) delay(500);   // CRITIQUE: 2 readings/s
    else if (val > 300) delay(1000);  // ATTENTION: 1 reading/s
    else                delay(2000);  // NORMAL  : 1 reading every 2 s
}
