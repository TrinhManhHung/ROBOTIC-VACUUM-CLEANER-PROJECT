#pragma once
#include <Arduino.h>

void batteryInit();

// Goi lien tuc trong loop(). Chi doc ADC moi BATT_INTERVAL_MS.
void batteryUpdate();

// Dien ap pin 3S da loc (V)
float batteryVoltage();

// Dung luong con lai uoc tinh (0..100 %)
uint8_t batteryPercent();

// Pin yeu (< BATT_LOW_PERCENT)
bool batteryLow();
