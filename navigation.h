#pragma once
#include "types.h"

void navInit();

// Goi lien tuc trong loop(), sau sensorsUpdate()
void navUpdate();

void navStartAuto();
void navStop();
void navManual(ManualCmd cmd);

RobotMode navGetMode();
NavState navGetState();
const char* navStateName(NavState s);
const char* navModeName(RobotMode m);
