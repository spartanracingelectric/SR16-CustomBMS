#include <hv_sense.h>
#include "adc.h"
#include "main.h"
#include <stdio.h>
#include "usart.h"

	void ReadHVInput(batteryModule *batt) {
		uint32_t adcValue = 0;

		HAL_ADC_Start(&hadc1);//start adc with adc1
		if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
			adcValue = HAL_ADC_GetValue(&hadc1);
		}
		HAL_ADC_Stop(&hadc1);

		//calculate voltage based on  resolution and gain on opamp, voltage divider ratio
		float adcVoltage = ((float)adcValue / ADC_RESOLUTION) * 3.28;
		printf("adcVoltage for hv is: %f\n", adcVoltage);
		float amcOutput = adcVoltage / GAIN_TLV9001;
		float hvInput = (amcOutput) * (DIVIDER_RATIO) + .9;

		batt->pack_voltage = hvInput * 100;
	}

