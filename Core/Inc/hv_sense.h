#ifndef INC_HV_SENSE_H_
#define INC_HV_SENSE_H_

#include "main.h"

#define GAIN_TLV9001 1.58f 		//TLV9001 gain
//Resistor values for the voltage divider
#define R1 1400000.0f  			//1.4 MΩ
#define R2 6761.5f     			//6863kΩ is the spec, but it might be parallel to other components. measured value was 6660kΩ, but it was too low, so I took average of them and it works well.
#define DIVIDER_RATIO (R1 + R2) / R2
#define PRECHARGE_DONE_RATIO 	0.90f
#define PRECHARGE_MIN_PACK_V 	60.0f
#define PRECHARGE_TIMEOUT_MS 	5000

#define VREFINT_CALIBRATION_ADC_RAW_COUNT 16
#define VREFINT_CALIBRATION_SUPPLY_POWER_MV 3300

typedef enum {
	PRECHARGE_IDLE = 0,	//no request from the VCU, both outputs open
	PRECHARGE_ACTIVE,	//precharge relay closed, waiting for the tractive side to charge
	PRECHARGE_DONE,		//tractive side charged, contactor closed
	PRECHARGE_FAULT
} PrechargeState;

void ReadHVInput(batteryModule *batt);

void getSumPackVoltage(batteryModule *batt);

#endif /* INC_HV_SENSE_H_ */
