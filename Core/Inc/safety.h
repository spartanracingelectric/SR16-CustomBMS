#ifndef INC_SAFETY_H_
#define INC_SAFETY_H_

#include "main.h"

void Cell_Summary_Voltage(struct batteryModule *batt, uint8_t *fault,
                          uint8_t *warnings, uint8_t *states,
                          uint8_t *low_volt_hysteresis,
                          uint8_t *high_volt_hysteresis,
                          uint8_t *cell_imbalance_hysteresis);

void Cell_Summary_Temperature(struct batteryModule *batt, uint8_t *fault,
		uint8_t *warnings);
void Module_Averages(struct batteryModule *batt);

#endif /* INC_SAFETY_H_ */
