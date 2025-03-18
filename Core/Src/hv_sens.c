#include <hv_sense.h>
#include "adc.h"
#include "main.h"
#include <stdio.h>
#include "usart.h"

	void ReadHVInput(batteryModule *batt) {
		uint32_t adcValue = 0;

		HAL_ADC_Start(&hadc1);//start adc with adc1
		if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
			adcValue = HAL_ADC_GetValue(&hadc1);//get adc value and store it in adcValue
//			adcValue += 18 ; // offset(found when input is 0v)
//			printf("adc for hv is: %d\n", adcValue);
		}
		HAL_ADC_Stop(&hadc1);//stop adc

		//calculate voltage based on  resolution and gain on opamp, voltage divider ratio
		float adcVoltage = ((float)adcValue / ADC_RESOLUTION) * 3.28;
		printf("adcVoltage for hv is: %f\n", adcVoltage);
		float amcOutput = adcVoltage / GAIN_TLV9001;
		float hvInput = (amcOutput) * (DIVIDER_RATIO) + .9;

		batt->pack_voltage = hvInput * 100;
	}

	void State_of_Charge(batteryModule *batt, uint32_t elapsed_time) {
	    uint32_t adcValue = 0;
		HAL_ADC_Start(&hadc2);//start adc with adc1
		if (HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY) == HAL_OK) {
			adcValue = HAL_ADC_GetValue(&hadc2);//get adc value and store it in adcValue
		}
		HAL_ADC_Stop(&hadc2);//stop adc
	    float voltage = ((float)adcValue / ADC_RESOLUTION) * V_REF;
	    batt->current = (voltage / MAX_SHUNT_VOLTAGE) * MAX_SHUNT_AMPAGE;
	    batt->soc -= (uint16_t)(batt->current * (float)(elapsed_time / 3600000.0f));
	}

