#include "motor.h"
#include "config.h"
#include <esp_arduino_version.h>

namespace {

struct Wheel {
  uint8_t in1, in2, en, ch;
  bool invert;
  float trim;
  int target;
  int current;
  uint32_t kickUntil;
};

Wheel wheelL = {PIN_IN1, PIN_IN2, PIN_ENA, 0, MOTOR_LEFT_INVERT,  MOTOR_LEFT_TRIM,  0, 0, 0};
Wheel wheelR = {PIN_IN3, PIN_IN4, PIN_ENB, 1, MOTOR_RIGHT_INVERT, MOTOR_RIGHT_TRIM, 0, 0, 0};

bool braking = false;
uint32_t lastRampMs = 0;

// Ho tro ca ESP32 Arduino core 2.x va 3.x
void pwmAttach(const Wheel& w) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(w.en, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
#else
  ledcSetup(w.ch, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttachPin(w.en, w.ch);
#endif
}

void pwmWrite(const Wheel& w, uint32_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(w.en, duty);
#else
  ledcWrite(w.ch, duty);
#endif
}

void applyOutput(const Wheel& w, uint32_t now) {
  int speed = w.invert ? -w.current : w.current;

  if (speed == 0) {
    if (braking) {
      // IN1 = IN2 = HIGH, EN = HIGH -> phanh nhanh
      digitalWrite(w.in1, HIGH);
      digitalWrite(w.in2, HIGH);
      pwmWrite(w, 255);
    } else {
      digitalWrite(w.in1, LOW);
      digitalWrite(w.in2, LOW);
      pwmWrite(w, 0);
    }
    return;
  }

  digitalWrite(w.in1, speed > 0 ? HIGH : LOW);
  digitalWrite(w.in2, speed > 0 ? LOW : HIGH);

  int duty = max(abs(speed), MOTOR_MIN_PWM);
  if (now < w.kickUntil) duty = max(duty, MOTOR_KICK_PWM);
  duty = constrain((int)(duty * w.trim), 0, 255);
  pwmWrite(w, duty);
}

void rampWheel(Wheel& w, uint32_t now) {
  int prev = w.current;
  if (prev == 0) {
    // Tu dung yen: dat ngay toc do muc tieu (kick o duoi giup motor thang ma sat tinh).
    // Neu tang tu tu tu 0, motor bi ket o MOTOR_MIN_PWM qua lau -> khong lui/quay noi.
    w.current = w.target;
  } else if (w.current < w.target) {
    w.current = min(w.current + MOTOR_RAMP_STEP, w.target);
  } else if (w.current > w.target) {
    w.current = max(w.current - MOTOR_RAMP_STEP, w.target);
  }

  // Bat dau quay tu dung yen hoac doi chieu -> kick de thang ma sat tinh
  if (w.current != 0 && (prev == 0 || (prev > 0) != (w.current > 0))) {
    w.kickUntil = now + MOTOR_KICK_MS;
  }
}

}  // namespace

void motorInit() {
  for (Wheel* w : {&wheelL, &wheelR}) {
    pinMode(w->in1, OUTPUT);
    pinMode(w->in2, OUTPUT);
    digitalWrite(w->in1, LOW);
    digitalWrite(w->in2, LOW);
    pwmAttach(*w);
    pwmWrite(*w, 0);
  }
}

void motorSet(int left, int right) {
  braking = false;
  wheelL.target = constrain(left, -255, 255);
  wheelR.target = constrain(right, -255, 255);
}

void motorStop() {
  motorSet(0, 0);
}

void motorBrake() {
  braking = true;
  uint32_t now = millis();
  for (Wheel* w : {&wheelL, &wheelR}) {
    w->target = 0;
    w->current = 0;
    applyOutput(*w, now);
  }
}

void motorUpdate() {
  uint32_t now = millis();
  if (now - lastRampMs < MOTOR_RAMP_INTERVAL_MS) return;
  lastRampMs = now;

  rampWheel(wheelL, now);
  rampWheel(wheelR, now);
  applyOutput(wheelL, now);
  applyOutput(wheelR, now);
}

int motorGetLeft()  { return wheelL.current; }
int motorGetRight() { return wheelR.current; }
