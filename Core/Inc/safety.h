#ifndef INC_SAFETY_H_
#define INC_SAFETY_H_

#include <stdint.h>

#include "module.h"

void Cell_Voltage_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings, uint8_t *states,
                        uint8_t *high_volt_fault_lock,  uint8_t *low_volt_hysteresis, uint8_t *low_volt_fault_lock,
                        uint8_t *cell_imbalance_hysteresis);

void Cell_Temperature_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings, uint8_t *high_temp_hysteresis);

void High_Voltage_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings);

void Module_Voltage_Averages(struct batteryModule *batt);

void Module_Temperature_Averages(struct batteryModule *batt);

#endif /* INC_SAFETY_H_ */
