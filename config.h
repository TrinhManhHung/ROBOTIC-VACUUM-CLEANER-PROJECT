#pragma once

// ============================================================
//  CHAN KET NOI
// ============================================================

// --- L298N ---
// Motor A (ENA/IN1/IN2) = banh TRAI, Motor B (ENB/IN3/IN4) = banh PHAI
#define PIN_ENA 13
#define PIN_IN1 12
#define PIN_IN2 14
#define PIN_IN3 27
#define PIN_IN4 26
#define PIN_ENB 25

// --- HC-SR04 ---
// Luu y: GPIO34/35/36/39 chi la chan INPUT, khong dung lam Trig duoc.
// Vi vay cam bien PHAI dung D21 (Trig) / D19 (Echo).
#define PIN_TRIG_CENTER 33
#define PIN_ECHO_CENTER 32
#define PIN_TRIG_RIGHT  21
#define PIN_ECHO_RIGHT  19
#define PIN_TRIG_LEFT   23
#define PIN_ECHO_LEFT   22

// ============================================================
//  DONG CO
// ============================================================

#define MOTOR_PWM_FREQ 1000   // Hz - L298N chay tot o tan so thap
#define MOTOR_PWM_RES  8      // bit -> duty 0..255

// Dao chieu neu banh quay nguoc so voi mong muon (do dau day)
#define MOTOR_LEFT_INVERT  false
#define MOTOR_RIGHT_INVERT false

// Bu lech toc do 2 banh de robot di thang (0.0 .. 1.0)
#define MOTOR_LEFT_TRIM  1.00f
#define MOTOR_RIGHT_TRIM 1.00f

// PWM toi thieu de motor bat dau quay (vung chet). Can chinh theo motor thuc te.
#define MOTOR_MIN_PWM  75
// "Moi" motor khi khoi dong tu dung yen / doi chieu
#define MOTOR_KICK_PWM 200
#define MOTOR_KICK_MS  100

// Tang/giam toc tu tu: moi MOTOR_RAMP_INTERVAL_MS thay doi MOTOR_RAMP_STEP
#define MOTOR_RAMP_INTERVAL_MS 10
#define MOTOR_RAMP_STEP        3

// ============================================================
//  TOC DO (0..255) - chay cham de hut bui
// ============================================================

// Neu motor khong quay noi -> tang MOTOR_MIN_PWM; muon cham hon -> giam SPEED_CRUISE
// (gia tri nho hon MOTOR_MIN_PWM se tu dong duoc nang len MOTOR_MIN_PWM)
#define SPEED_CRUISE  95    // chay thang binh thuong (cham de hut bui)
#define SPEED_SLOW    80    // khi phia truoc co vat can o gan
// Lui/quay chi dien ra ngan khi tranh vat can -> can luc manh hon di thang
// (quay tai cho banh bi truot ngang nen ton luc nhat)
#define SPEED_TURN    140   // quay tai cho
#define SPEED_BACK    120   // lui
#define SPEED_MANUAL  100   // dieu khien tay

#define STEER_FACTOR  0.6f  // banh phia trong khi lach tranh tuong

// ============================================================
//  CAM BIEN SIEU AM
// ============================================================

#define SONAR_INTERVAL_MS 30      // khoang cach giua 2 lan do (tranh nhieu cheo)
#define SONAR_TIMEOUT_US  15000   // ~2.5 m
#define SONAR_MAX_CM      250.0f  // khong co echo -> coi nhu thoang
#define SONAR_MIN_CM      2.0f

// ============================================================
//  NGUONG TRANH VAT CAN (cm)
// ============================================================

#define DIST_STOP_CENTER   15   // phia truoc < nguong nay -> dung, lui, quay
#define DIST_SLOW_CENTER   35   // phia truoc < nguong nay -> giam toc
#define DIST_CLEAR_CENTER  30   // dang quay, phia truoc > nguong nay -> di tiep
#define DIST_STOP_SIDE     10   // ben canh qua sat -> xu ly nhu vat can
#define DIST_STEER_SIDE    18   // ben canh gan -> lach nhe ra xa
#define DIST_SIDE_EQUAL    10   // trai/phai chenh nhau it hon -> giu huong quay cu

// ============================================================
//  THOI GIAN XU LY VAT CAN (ms)
// ============================================================

#define BRAKE_MS        150
#define BACKUP_MS       500
#define TURN_MIN_MS     400
#define TURN_MAX_MS     2000
#define ESCAPE_TURN_MS  1500   // ~180 do, can chinh theo robot

// Gap vat can STUCK_COUNT lan trong STUCK_WINDOW_MS -> coi nhu ket goc
#define STUCK_COUNT      4
#define STUCK_WINDOW_MS  6000

// ============================================================
//  KHAC
// ============================================================

#define SERIAL_BAUD        115200
#define DEBUG_INTERVAL_MS  500
#define AUTO_START_ON_BOOT true    // true: tu chay AUTO sau khi bat nguon
#define AUTO_START_DELAY_MS 3000
