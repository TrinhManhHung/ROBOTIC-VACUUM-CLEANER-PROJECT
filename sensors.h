#pragma once
#include "types.h"

void sensorsInit();

// Goi lien tuc trong loop(). Moi lan chi do 1 cam bien (luan phien) de tranh nhieu cheo.
void sensorsUpdate();

// Khoang cach da loc (cm). Khong co echo -> SONAR_MAX_CM.
const Distances& sensorsGet();

// Trang thai cam bien chong roi TCRT5000. Duoc doc moi lan goi sensorsUpdate().
const CliffState& sensorsCliff();
