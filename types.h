#pragma once
#include <Arduino.h>

struct Distances {
  float left;
  float center;
  float right;
};

enum RobotMode : uint8_t {
  MODE_IDLE,
  MODE_AUTO,
  MODE_MANUAL,
};

enum NavState : uint8_t {
  NAV_STOPPED,
  NAV_FORWARD,
  NAV_BRAKE,
  NAV_BACKUP,
  NAV_TURN,
  NAV_ESCAPE,
};

enum TurnDir : uint8_t {
  TURN_LEFT,
  TURN_RIGHT,
};

enum ManualCmd : uint8_t {
  MANUAL_STOP,
  MANUAL_FORWARD,
  MANUAL_BACKWARD,
  MANUAL_LEFT,
  MANUAL_RIGHT,
};
