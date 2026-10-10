#include "navigation.h"
#include "config.h"
#include "motor.h"
#include "sensors.h"

namespace {

RobotMode mode = MODE_IDLE;
NavState state = NAV_STOPPED;
uint32_t stateStartMs = 0;

TurnDir turnDir = TURN_RIGHT;
bool escapeAfterBackup = false;

// Lan xu ly hien tai la do mep roi (khong phai vat can sieu am)
bool cliffEvent = false;
uint32_t lastCliffMs = 0;

ManualCmd manualCmd = MANUAL_STOP;

// Lich su cac lan gap vat can de phat hien bi ket trong goc
uint32_t obstacleTimes[STUCK_COUNT] = {0};
uint8_t obstacleIdx = 0;

void setState(NavState s) {
  if (s == state) return;
  state = s;
  stateStartMs = millis();
  Serial.printf("[NAV] -> %s\n", navStateName(s));
}

// Ghi nhan 1 lan gap vat can, tra ve true neu dang bi ket
bool registerObstacle(uint32_t now) {
  obstacleTimes[obstacleIdx] = now;
  obstacleIdx = (obstacleIdx + 1) % STUCK_COUNT;
  // Phan tu cu nhat nam ngay tai obstacleIdx
  uint32_t oldest = obstacleTimes[obstacleIdx];
  if (oldest != 0 && now - oldest < STUCK_WINDOW_MS) {
    memset(obstacleTimes, 0, sizeof(obstacleTimes));
    return true;
  }
  return false;
}

TurnDir chooseTurn(const Distances& d, bool blockedL, bool blockedR) {
  if (blockedL && !blockedR) return TURN_RIGHT;
  if (blockedR && !blockedL) return TURN_LEFT;
  // Hai ben xap xi nhau -> giu huong cu de khong lac qua lac lai trong goc
  if (fabsf(d.left - d.right) < DIST_SIDE_EQUAL) return turnDir;
  return d.left > d.right ? TURN_LEFT : TURN_RIGHT;
}

void spin(TurnDir dir) {
  if (dir == TURN_LEFT) motorSet(-SPEED_TURN, SPEED_TURN);
  else                  motorSet(SPEED_TURN, -SPEED_TURN);
}

bool frontClear(const Distances& d) {
  return d.center > DIST_CLEAR_CENTER &&
         d.left > DIST_STOP_SIDE && d.right > DIST_STOP_SIDE;
}

void updateAuto(const Distances& d, bool cliff, uint32_t now) {
  if (cliff) lastCliffMs = now;

  // Nguy co roi uu tien cao nhat: dung ngay o moi trang thai (tru khi dang phanh/lui ra xa mep)
  if (cliff && state != NAV_BRAKE && state != NAV_BACKUP) {
    motorBrake();
    cliffEvent = true;
    turnDir = chooseTurn(d, false, false);
    escapeAfterBackup = registerObstacle(now);
    Serial.printf("[NAV] Mep roi khi dang %s -> lui, quay %s%s\n",
                  navStateName(state),
                  turnDir == TURN_LEFT ? "TRAI" : "PHAI",
                  escapeAfterBackup ? " (KET GOC)" : "");
    setState(NAV_BRAKE);
    return;
  }

  uint32_t elapsed = now - stateStartMs;

  switch (state) {
    case NAV_STOPPED:
      setState(NAV_FORWARD);
      break;

    case NAV_FORWARD: {
      bool blockedC = d.center < DIST_STOP_CENTER;
      bool blockedL = d.left < DIST_STOP_SIDE;
      bool blockedR = d.right < DIST_STOP_SIDE;

      if (blockedC || blockedL || blockedR) {
        motorBrake();
        cliffEvent = false;
        turnDir = chooseTurn(d, blockedL, blockedR);
        escapeAfterBackup = registerObstacle(now);
        Serial.printf("[NAV] Vat can L=%.0f C=%.0f R=%.0f -> quay %s%s\n",
                      d.left, d.center, d.right,
                      turnDir == TURN_LEFT ? "TRAI" : "PHAI",
                      escapeAfterBackup ? " (KET GOC)" : "");
        setState(NAV_BRAKE);
        break;
      }

      int base = d.center < DIST_SLOW_CENTER ? SPEED_SLOW : SPEED_CRUISE;
      int l = base, r = base;
      // Lach nhe ra xa tuong ben canh
      if (d.left < DIST_STEER_SIDE && d.left <= d.right)        r = base * STEER_FACTOR;
      else if (d.right < DIST_STEER_SIDE && d.right < d.left)   l = base * STEER_FACTOR;
      motorSet(l, r);
      break;
    }

    case NAV_BRAKE:
      if (elapsed >= BRAKE_MS) setState(NAV_BACKUP);
      break;

    case NAV_BACKUP:
      motorSet(-SPEED_BACK, -SPEED_BACK);
      if (cliff) {
        // Van chua thay san: lui tiep; qua lau -> co the bi nhac len, dung han cho an toan
        if (elapsed >= CLIFF_BACKUP_MAX_MS) {
          motorBrake();
          mode = MODE_IDLE;
          setState(NAV_STOPPED);
          Serial.println("[NAV] Lui qua lau van khong thay san (bi nhac len?) -> DUNG");
        }
      } else if (elapsed >= BACKUP_MS &&
                 (!cliffEvent || now - lastCliffMs >= CLIFF_EXTRA_BACKUP_MS)) {
        motorBrake();
        setState(escapeAfterBackup ? NAV_ESCAPE : NAV_TURN);
      }
      break;

    case NAV_TURN:
      spin(turnDir);
      if (cliffEvent) {
        // Sieu am thay phia truoc thoang ca khi do la mep roi -> quay du goc co dinh
        if (elapsed >= CLIFF_TURN_MS) {
          motorBrake();
          cliffEvent = false;
          setState(NAV_FORWARD);
        }
      } else if ((elapsed >= TURN_MIN_MS && frontClear(d)) || elapsed >= TURN_MAX_MS) {
        // Het TURN_MAX_MS ma van chua thoang: NAV_FORWARD se phat hien lai
        // va tiep tuc lui/quay; lap lai nhieu lan se kich hoat ESCAPE.
        if (!frontClear(d)) {
          Serial.printf("[NAV] Quay het %d ms van chua thoang L=%.0f C=%.0f R=%.0f\n",
                        TURN_MAX_MS, d.left, d.center, d.right);
        }
        motorBrake();
        setState(NAV_FORWARD);
      }
      break;

    case NAV_ESCAPE:
      spin(turnDir);
      if (elapsed >= ESCAPE_TURN_MS) {
        motorBrake();
        // Lan sau doi huong de khong lap lai cung mot kieu thoat
        turnDir = turnDir == TURN_LEFT ? TURN_RIGHT : TURN_LEFT;
        setState(NAV_FORWARD);
      }
      break;
  }
}

void updateManual(const Distances& d, bool cliff) {
  switch (manualCmd) {
    case MANUAL_FORWARD:
      if (cliff) {
        motorBrake();
        manualCmd = MANUAL_STOP;
        Serial.println("[NAV] Dieu khien tay: phia truoc la mep roi, da dung (chi cho lui)");
      } else if (d.center < DIST_STOP_CENTER) {
        motorBrake();
        manualCmd = MANUAL_STOP;
        Serial.println("[NAV] Dieu khien tay: phia truoc co vat can, da dung");
      } else {
        motorSet(SPEED_MANUAL, SPEED_MANUAL);
      }
      break;
    case MANUAL_BACKWARD: motorSet(-SPEED_MANUAL, -SPEED_MANUAL); break;
    case MANUAL_LEFT:     motorSet(-SPEED_TURN, SPEED_TURN);      break;
    case MANUAL_RIGHT:    motorSet(SPEED_TURN, -SPEED_TURN);      break;
    case MANUAL_STOP:     motorStop();                            break;
  }
}

}  // namespace

void navInit() {
  mode = MODE_IDLE;
  state = NAV_STOPPED;
  motorBrake();
}

void navUpdate() {
  const Distances& d = sensorsGet();
  bool cliff = sensorsCliff().detected;
  uint32_t now = millis();

  switch (mode) {
    case MODE_AUTO:   updateAuto(d, cliff, now); break;
    case MODE_MANUAL: updateManual(d, cliff);    break;
    case MODE_IDLE:   break;
  }
}

void navStartAuto() {
  mode = MODE_AUTO;
  cliffEvent = false;
  memset(obstacleTimes, 0, sizeof(obstacleTimes));
  setState(NAV_FORWARD);
  Serial.println("[NAV] Che do AUTO");
}

void navStop() {
  mode = MODE_IDLE;
  manualCmd = MANUAL_STOP;
  motorBrake();
  setState(NAV_STOPPED);
  Serial.println("[NAV] Dung");
}

void navManual(ManualCmd cmd) {
  if (mode != MODE_MANUAL) {
    mode = MODE_MANUAL;
    setState(NAV_STOPPED);
    Serial.println("[NAV] Che do MANUAL");
  }
  manualCmd = cmd;
}

RobotMode navGetMode()  { return mode; }
NavState  navGetState() { return state; }

const char* navStateName(NavState s) {
  switch (s) {
    case NAV_STOPPED: return "STOPPED";
    case NAV_FORWARD: return "FORWARD";
    case NAV_BRAKE:   return "BRAKE";
    case NAV_BACKUP:  return "BACKUP";
    case NAV_TURN:    return "TURN";
    case NAV_ESCAPE:  return "ESCAPE";
  }
  return "?";
}

const char* navModeName(RobotMode m) {
  switch (m) {
    case MODE_IDLE:   return "IDLE";
    case MODE_AUTO:   return "AUTO";
    case MODE_MANUAL: return "MANUAL";
  }
  return "?";
}
