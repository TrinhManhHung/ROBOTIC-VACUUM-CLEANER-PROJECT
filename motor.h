#pragma once
#include <Arduino.h>

void motorInit();

// Dat toc do muc tieu cho 2 banh (-255..255). Toc do thuc te tang/giam tu tu.
void motorSet(int left, int right);

// Giam toc tu tu ve 0 (tha troi)
void motorStop();

// Dung ngay lap tuc (phanh chu dong) - dung khi gap vat can
void motorBrake();

// Goi lien tuc trong loop()
void motorUpdate();

int motorGetLeft();
int motorGetRight();
