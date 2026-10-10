// Robot hut bui / lau nha - ESP32 + L298N + 3x HC-SR04 + TCRT5000
// Giai doan 1: di chuyen cham, tranh vat can va chong roi
//
// Lenh qua Serial Monitor (115200, gui tung ky tu):
//   g : bat dau che do AUTO (tu di, tranh vat can)
//   x : dung
//   w/s/a/d : dieu khien tay tien / lui / trai / phai
//   i : bat/tat in thong tin cam bien

#include <esp_system.h>
#include "config.h"
#include "types.h"
#include "motor.h"
#include "sensors.h"
#include "navigation.h"

static bool debugEnabled = true;
static uint32_t lastDebugMs = 0;
static bool autoStartPending = AUTO_START_ON_BOOT;

// Bo nho RTC giu nguyen qua reset (tru khi mat nguon hoan toan)
// -> biet truoc khi reset robot co dang chay AUTO khong
#define AUTO_MAGIC        0xA070A070
#define MAX_AUTO_RESUMES  3
RTC_NOINIT_ATTR static uint32_t rtcAutoMagic;
RTC_NOINIT_ATTR static uint32_t rtcResumeCount;

static bool isAbnormalReset(esp_reset_reason_t r) {
  return r == ESP_RST_BROWNOUT || r == ESP_RST_PANIC ||
         r == ESP_RST_INT_WDT  || r == ESP_RST_TASK_WDT || r == ESP_RST_WDT;
}

static void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'g': case 'G': autoStartPending = false; rtcResumeCount = 0; navStartAuto(); break;
      case 'x': case 'X': autoStartPending = false; navStop();      break;
      case 'w': case 'W': navManual(MANUAL_FORWARD);  break;
      case 's': case 'S': navManual(MANUAL_BACKWARD); break;
      case 'a': case 'A': navManual(MANUAL_LEFT);     break;
      case 'd': case 'D': navManual(MANUAL_RIGHT);    break;
      case 'i': case 'I': debugEnabled = !debugEnabled; break;
      default: break;
    }
  }
}

static void printDebug() {
  uint32_t now = millis();
  if (!debugEnabled || now - lastDebugMs < DEBUG_INTERVAL_MS) return;
  lastDebugMs = now;

  const Distances& d = sensorsGet();
  const CliffState& c = sensorsCliff();
  Serial.printf("L=%5.1f C=%5.1f R=%5.1f cm | Roi=%s D0=%d | %s/%s | motor L=%4d R=%4d\n",
                d.left, d.center, d.right,
                c.detected ? "CO" : "--", digitalRead(PIN_CLIFF_DO),
                navModeName(navGetMode()), navStateName(navGetState()),
                motorGetLeft(), motorGetRight());
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(200);
  Serial.println("\n=== Robot lau nha - khoi dong ===");

  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Ly do khoi dong: %d\n", (int)reason);
  if (reason == ESP_RST_BROWNOUT) {
    Serial.println("!!! CANH BAO: ESP32 vua bi reset do SUT AP (brownout).");
    Serial.println("!!! Thuong do motor keo dong lam sut nguon 5V. Xem huong dan sua phan cung.");
  }

  if (isAbnormalReset(reason) && rtcAutoMagic == AUTO_MAGIC) {
    if (rtcResumeCount < MAX_AUTO_RESUMES) {
      rtcResumeCount++;
      autoStartPending = true;
      Serial.printf("!!! Dang chay AUTO thi bi reset -> tu chay lai (lan %u/%d)\n",
                    (unsigned)rtcResumeCount, MAX_AUTO_RESUMES);
    } else {
      autoStartPending = false;  // ke ca khi AUTO_START_ON_BOOT = true
      Serial.println("!!! Reset lien tuc qua nhieu lan -> KHONG tu chay lai. Kiem tra nguon!");
    }
  } else {
    rtcResumeCount = 0;
  }
  rtcAutoMagic = 0;

  motorInit();
  sensorsInit();
  navInit();

  Serial.println("Lenh: g=AUTO, x=dung, w/s/a/d=dieu khien tay, i=bat/tat debug");
}

void loop() {
  handleSerial();

  if (autoStartPending && millis() >= AUTO_START_DELAY_MS) {
    autoStartPending = false;
    navStartAuto();
  }

  sensorsUpdate();
  navUpdate();
  motorUpdate();
  printDebug();

  rtcAutoMagic = navGetMode() == MODE_AUTO ? AUTO_MAGIC : 0;
}
