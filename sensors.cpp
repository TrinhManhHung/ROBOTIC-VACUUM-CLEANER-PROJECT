#include "sensors.h"
#include "config.h"

namespace {

enum SonarId : uint8_t { SONAR_LEFT, SONAR_CENTER, SONAR_RIGHT, SONAR_COUNT };

struct Sonar {
  uint8_t trig, echo;
  float hist[3];
  uint8_t idx;
};

Sonar sonars[SONAR_COUNT] = {
  {PIN_TRIG_LEFT,   PIN_ECHO_LEFT,   {SONAR_MAX_CM, SONAR_MAX_CM, SONAR_MAX_CM}, 0},
  {PIN_TRIG_CENTER, PIN_ECHO_CENTER, {SONAR_MAX_CM, SONAR_MAX_CM, SONAR_MAX_CM}, 0},
  {PIN_TRIG_RIGHT,  PIN_ECHO_RIGHT,  {SONAR_MAX_CM, SONAR_MAX_CM, SONAR_MAX_CM}, 0},
};

// Cam bien giua duoc do gap doi vi quan trong nhat khi di thang
const SonarId ORDER[] = {SONAR_CENTER, SONAR_LEFT, SONAR_CENTER, SONAR_RIGHT};
const uint8_t ORDER_LEN = sizeof(ORDER) / sizeof(ORDER[0]);

uint8_t orderIdx = 0;
uint32_t lastMeasureMs = 0;
Distances data = {SONAR_MAX_CM, SONAR_MAX_CM, SONAR_MAX_CM};

float measureCm(const Sonar& s) {
  digitalWrite(s.trig, LOW);
  delayMicroseconds(2);
  digitalWrite(s.trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(s.trig, LOW);

  unsigned long us = pulseIn(s.echo, HIGH, SONAR_TIMEOUT_US);
  if (us == 0) return SONAR_MAX_CM;

  float cm = us / 58.0f;
  return constrain(cm, SONAR_MIN_CM, SONAR_MAX_CM);
}

float median3(float a, float b, float c) {
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}

}  // namespace

void sensorsInit() {
  for (Sonar& s : sonars) {
    pinMode(s.trig, OUTPUT);
    digitalWrite(s.trig, LOW);
    pinMode(s.echo, INPUT);
  }
}

void sensorsUpdate() {
  uint32_t now = millis();
  if (now - lastMeasureMs < SONAR_INTERVAL_MS) return;
  lastMeasureMs = now;

  SonarId id = ORDER[orderIdx];
  orderIdx = (orderIdx + 1) % ORDER_LEN;

  Sonar& s = sonars[id];
  s.hist[s.idx] = measureCm(s);
  s.idx = (s.idx + 1) % 3;
  float filtered = median3(s.hist[0], s.hist[1], s.hist[2]);

  switch (id) {
    case SONAR_LEFT:   data.left = filtered;   break;
    case SONAR_CENTER: data.center = filtered; break;
    case SONAR_RIGHT:  data.right = filtered;  break;
    default: break;
  }
}

const Distances& sensorsGet() {
  return data;
}
