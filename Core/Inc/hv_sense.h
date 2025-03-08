#ifndef INC_HV_SENSE_H_
#define INC_HV_SENSE_H_

#include "main.h"

#define ADC_RESOLUTION 4096.0f 	//12-bit ADC
#define V_REF 3.28f  			//Reference voltage (V)
#define GAIN_TLV9001 1.58f 		//TLV9001 gain
//Resistor values for the voltage divider
#define R1 1409560.0f  			//1.4 MΩ
#define R2 6863.f     			//6 863kΩ
#define DIVIDER_RATIO R1 / R2

#define MAX_SHUNT_AMPAGE 500000
#define MAX_SHUNT_VOLTAGE 7.6732f

void ReadHVInput(batteryModule *batt);

void State_of_Charge(batteryModule *batt, uint32_t elapsed_time);

#endif /* INC_HV_SENSE_H_ */
