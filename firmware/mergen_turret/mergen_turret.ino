#include <stdio.h> //scanf
#include <stdlib.h> //abs()

// pin definitions for tmc2208 and laser
// definitions to use less ram
#define STEP_X 2
#define DIR_X  3
#define STEP_Y 4
#define DIR_Y  5
#define EN_PIN 6
#define LASER_PIN 11
#define BRUDRATE 115200

// deadzone and speed limits (in microseconds)
#define DEADZONE 35        // to disallow vibrations on steps
#define MIN_DELAY_US 600   // fastest step speed
#define MAX_DELAY_US 4000  // slowest step speed
#define TIMEOUT_MS 500     // stop if pc disconnects

// invert motor directions if moving backwards (0 or 1)
#define INVERT_X 0
#define INVERT_Y 0

// target state from pc
int dx = 0;
int dy = 0;
int fire = 0;

// serial buffer
// reciever
char rx_buf[32];
int buf_idx = 0;

// timers
unsigned long last_rx_time = 0;
unsigned long last_step_x = 0;
unsigned long last_step_y = 0;

// convert pixel error to step delay (bigger error = faster steps)
unsigned long get_step_delay(int err) {
    int abs_err = abs(err);
    if (abs_err > 300) abs_err = 300;

    long delay_us = MAX_DELAY_US - ((long)(abs_err - DEADZONE) * (MAX_DELAY_US - MIN_DELAY_US)) / (300 - DEADZONE);

    if (delay_us < MIN_DELAY_US) delay_us = MIN_DELAY_US;
    if (delay_us > MAX_DELAY_US) delay_us = MAX_DELAY_US;

    return (unsigned long)delay_us;
}

// read "<dx,dy,fire>\n" packet from pc
void read_serial() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();

        if (c == '<') {
            // start of new packet
            buf_idx = 0;
            rx_buf[buf_idx++] = c;
        } else if (buf_idx > 0 && buf_idx < (int)sizeof(rx_buf) - 1) {
            rx_buf[buf_idx++] = c;

            if (c == '>') {
                rx_buf[buf_idx] = '\0';
                int new_dx = 0, new_dy = 0, new_fire = 0;

                // parse c string
                if (sscanf(rx_buf, "<%d,%d,%d>", &new_dx, &new_dy, &new_fire) == 3) {
                    dx = new_dx;
                    dy = new_dy;
                    fire = new_fire;
                    last_rx_time = millis();
                }
                buf_idx = 0;
            }
        } else {
            buf_idx = 0;
        }
    }
}

// step x and y motors without blocking
void move_motors() {
    unsigned long now_us = micros();

    // x axis (left / right)
    if (abs(dx) > DEADZONE) {
        digitalWrite(DIR_X, ((dx > 0) ^ INVERT_X) ? HIGH : LOW);
        unsigned long delay_x = get_step_delay(dx);

        if (now_us - last_step_x >= delay_x) {
            last_step_x = now_us;
            digitalWrite(STEP_X, HIGH);
            delayMicroseconds(5); // nabız
            digitalWrite(STEP_X, LOW);
        }
    }

    // y axis (up / down)
    if (abs(dy) > DEADZONE) {
        digitalWrite(DIR_Y, ((dy > 0) ^ INVERT_Y) ? HIGH : LOW);
        unsigned long delay_y = get_step_delay(dy);

        if (now_us - last_step_y >= delay_y) {
            last_step_y = now_us;
            digitalWrite(STEP_Y, HIGH);
            delayMicroseconds(5); // nabız
            digitalWrite(STEP_Y, LOW);
        }
    }

    // laser control
    digitalWrite(LASER_PIN, (fire == 1) ? HIGH : LOW);
}

// stop everything if no data from pc for TIMEOUT_MS = 500ms
void check_safety() {
    if (millis() - last_rx_time > TIMEOUT_MS) {
        dx = 0;
        dy = 0;
        fire = 0;
        digitalWrite(LASER_PIN, LOW);
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
    digitalWrite(LASER_PIN, LOW);

    Serial.begin(BRUDRATE);
}

void loop() {
    read_serial();
    check_safety();
    move_motors();
}
