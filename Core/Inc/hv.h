#ifndef INC_HV_H_
#define INC_HV_H_

#include "main.h"

#define ADC_RESOLUTION 4096.0f 	//12-bit ADC
#define V_REF 3.3f  			//Reference voltage (V)
#define GAIN_TLV9001 1.58f 		//TLV9001 gain
//Resistor values for the voltage divider
#define R1 1400060.0f  			//1.4 MΩ
#define R2 6810.0f     			//6 863kΩ
#define DIVIDER_RATIO R1 / R2

void ReadHVInput(uint32_t *read_volt_HV);

void State_of_Charge(struct batteryModule *batt, uint8_t *fault,
		uint8_t *warnings);


#endif /* INC_HV_H_ */
