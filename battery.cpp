#include "battery.h"
#include "config.h"

namespace {

// Bang dien ap 1 cell Li-ion (khi nghi, khong tai) -> % dung luong
struct SocPoint { float v; uint8_t pct; };
const SocPoint SOC_TABLE[] = {
  {4.20f, 100}, {4.10f, 90}, {4.00f, 80}, {3.92f, 70}, {3.85f, 60},
  {3.79f,  50}, {3.75f, 40}, {3.71f, 30}, {3.67f, 20}, {3.61f, 10},
  {3.50f,   5}, {3.30f,  0},
};
const uint8_t SOC_LEN = sizeof(SOC_TABLE) / sizeof(SOC_TABLE[0]);

uint32_t lastReadMs = 0;
float voltage = 0;
uint8_t percent = 0;
bool low = false;

float readPackVoltage() {
  // analogReadMilliVolts() da dung so lieu hieu chuan eFuse cua ESP32 -> chinh xac hon analogRead()
  uint32_t sum = 0;
  for (uint8_t i = 0; i < BATT_SAMPLES; i++) sum += analogReadMilliVolts(PIN_BATT_SENSE);
  float pinV = sum / (float)BATT_SAMPLES / 1000.0f;
  return pinV * BATT_DIVIDER_RATIO * BATT_CALIB;
}

uint8_t cellToPercent(float cellV) {
  if (cellV >= SOC_TABLE[0].v) return 100;
  if (cellV <= SOC_TABLE[SOC_LEN - 1].v) return 0;
  for (uint8_t i = 1; i < SOC_LEN; i++) {
    const SocPoint& hi = SOC_TABLE[i - 1];
    const SocPoint& lo = SOC_TABLE[i];
    if (cellV >= lo.v) {
      float t = (cellV - lo.v) / (hi.v - lo.v);
      return (uint8_t)(lo.pct + t * (hi.pct - lo.pct) + 0.5f);
    }
  }
  return 0;
}

}  // namespace

void batteryInit() {
  pinMode(PIN_BATT_SENSE, INPUT);
  analogSetPinAttenuation(PIN_BATT_SENSE, ADC_11db);  // dai do ~0..3.1V
  voltage = readPackVoltage();
  percent = cellToPercent(voltage / BATT_CELLS);
}

void batteryUpdate() {
  uint32_t now = millis();
  if (now - lastReadMs < BATT_INTERVAL_MS) return;
  lastReadMs = now;

  // Loc mem: dien ap dao dong khi motor tang/giam toc
  voltage += BATT_FILTER_ALPHA * (readPackVoltage() - voltage);
  percent = cellToPercent(voltage / BATT_CELLS);

  // Co tre (hysteresis) de canh bao khong nhap nhay quanh nguong
  if (!low && percent < BATT_LOW_PERCENT) {
    low = true;
    Serial.printf("[BATT] PIN YEU: %.2fV (%u%%)\n", voltage, percent);
  } else if (low && percent >= BATT_LOW_PERCENT + 5) {
    low = false;
  }
}

float batteryVoltage() {
  return voltage;
}

uint8_t batteryPercent() {
  return percent;
}

bool batteryLow() {
  return low;
}
