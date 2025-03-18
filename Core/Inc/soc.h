#ifndef INC_SOC_H_
#define INC_SOC_H_

#include <stdlib.h>

#include "main.h"

#define MAX_SHUNT_AMPAGE 500000
#define MAX_SHUNT_VOLTAGE 2.62f

#define SHUNT_R1 24925.0f  // 24.9 kΩ
#define SHUNT_R2 20000.0f  // 20 kΩ
#define SHUNT_OPAMP_RATIO SHUNT_R1 / SHUNT_R2
#define SHUNT_OFFSET 500  // 500 mA

void SOC_getInitialCharge(batteryModule *batt);
void SOC_updateCharge(batteryModule *batt, uint32_t elapsed_time);

#endif
