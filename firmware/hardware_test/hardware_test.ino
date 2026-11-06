// pin definitions for tmc2208 and laser
#define STEP_X 2
#define DIR_X  3
#define STEP_Y 4
#define DIR_Y  5
#define EN_PIN 6
#define LASER_PIN 11

void step_motor(int step_pin, int steps) {
    for (int i = 0; i < steps; i++) {
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(1000);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(1000);
    }
}

void setup() {
    pinMode(STEP_X, OUTPUT);
    pinMode(DIR_X, OUTPUT);
    pinMode(STEP_Y, OUTPUT);
    pinMode(DIR_Y, OUTPUT);
    pinMode(EN_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);

    // enable tmc2208 drivers (active low)
    digitalWrite(EN_PIN, LOW);
}

void loop() {
    // x motor: 200 steps forward and backward (1 full revolution)
    digitalWrite(DIR_X, HIGH);
    step_motor(STEP_X, 200);
    delay(500);

    digitalWrite(DIR_X, LOW);
    step_motor(STEP_X, 200);
    delay(500);

    // y motor: 100 steps forward and backward
    digitalWrite(DIR_Y, HIGH);
    step_motor(STEP_Y, 100);
    delay(500);

    digitalWrite(DIR_Y, LOW);
    step_motor(STEP_Y, 100);
    delay(500);

    // laser test: 1 sec on, 1 sec off
    digitalWrite(LASER_PIN, HIGH);
    delay(1000);
    digitalWrite(LASER_PIN, LOW);
    delay(1000);
}
// X +1
// X -1
// Y +1/2
// Y -1/2
// Laser ON
// Laser OFF