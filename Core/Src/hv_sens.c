#include <hv_sense.h>
#include "adc.h"
#include "main.h"
#include <stdio.h>
#include "usart.h"

	void ReadHVInput(batteryModule *batt) {
		uint32_t adcValue = 0;
		float vRef = 0;

		adcValue = readADCChannel(ADC_CHANNEL_15);
		vRef = getVref();

		//calculate voltage based on  resolution and gain on opamp, voltage divider ratio
		float adcVoltage = ((float)adcValue / ADC_RESOLUTION) * vRef; //3.258 is the vref
//		printf("adcVoltage for hv is: %f\n", adcVoltage);
		float amcOutput = adcVoltage / GAIN_TLV9001;
		float hvInput = (amcOutput) * (DIVIDER_RATIO) + 0.215; //- .6840 is offset

		batt->hvsens_pack_voltage = hvInput * 100;
	}

	void getSumPackVoltage(batteryModule *batt){
		uint32_t sum_voltage = 0;

		for (int i = 0; i < NUM_CELLS; i++) {
			 sum_voltage += batt->cell_volt[i]; //get sum voltage
		}
		batt->sum_pack_voltage = (uint16_t)(sum_voltage / 100);
	}


