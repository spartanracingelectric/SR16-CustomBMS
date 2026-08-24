#include <hv_sense.h>
#include "adc.h"
#include "can.h"
#include "main.h"
#include <stdio.h>
#include "usart.h"

	//latched once precharge completes, so a voltage sag under load cannot re-close
	//the precharge relay onto an already closed contactor
	static uint8_t precharge_state = PRECHARGE_IDLE;
	static uint32_t precharge_start_ms = 0;

	void ReadHVInput(batteryModule *batt) {
		uint32_t adcValue = 0;
		float vRef = 0;

		adcValue = readADCChannel(ADC_CHANNEL_15);
		vRef = getVref();

		vRef = 3.3; // WORK AROUND SINCE VREF IS NOT WORKING AND IS CHANGING
//		printf("adcValue:%d\n", adcValue);

		//calculate voltage based on  resolution and gain on opamp, voltage divider ratio
		float adcVoltage = ((float)adcValue / ADC_RESOLUTION) * vRef;
//		printf("adcVoltage for hv is: %f\n", adcVoltage);
		float amcOutput = adcVoltage / GAIN_TLV9001;
		float hvInput = (amcOutput) * (DIVIDER_RATIO);
		if(hvInput > 10){//if hvsens is greater than 10V(connected)
			batt->hvsens_pack_voltage = hvInput * 100;
		}
		else{
			batt->hvsens_pack_voltage = 0;
		}

		uint32_t tractiveADC = readADCChannel(ADC_CHANNEL_14);
		float tractiveADCVolt = ((float)tractiveADC / ADC_RESOLUTION) * vRef;
		float tractiveAMCOut = tractiveADCVolt / GAIN_TLV9001;
		batt->tractive_voltage = (tractiveAMCOut) * (DIVIDER_RATIO);

		//the VCU asks for precharge over CAN, the BMS closes the precharge relay, then
		//hands over to the contactor once the tractive side has charged through the resistor
		float packVoltage = batt->sum_pack_voltage / 100.0f;

		if (!precharge_command) {
			precharge_state = PRECHARGE_IDLE;
		}
		else if (precharge_state == PRECHARGE_IDLE) {
			precharge_state = PRECHARGE_ACTIVE;
			precharge_start_ms = HAL_GetTick();
		}
		else if (precharge_state == PRECHARGE_ACTIVE
			  && packVoltage >= PRECHARGE_MIN_PACK_V
			  && (batt->hvsens_pack_voltage / 100) >= packVoltage * PRECHARGE_DONE_RATIO) {
			precharge_state = PRECHARGE_DONE;	//stays here until the VCU drops the request
		}
//		else if (precharge_state == PRECHARGE_ACTIVE
//			  && (HAL_GetTick() - precharge_start_ms) >= PRECHARGE_TIMEOUT_MS) {
//			precharge_state = PRECHARGE_FAULT;
//		}

		batt->precharge_status = precharge_state;
		HAL_GPIO_WritePin(MCU_PRECHARGE_SIGNAL_GPIO_Port, MCU_PRECHARGE_SIGNAL_Pin,
						  (precharge_state == PRECHARGE_ACTIVE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(MCU_CONTACTOR_SIGNAL_GPIO_Port, MCU_CONTACTOR_SIGNAL_Pin,
						  (precharge_state == PRECHARGE_DONE)   ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}

	void getSumPackVoltage(batteryModule *batt){
		uint32_t sum_voltage = 0;

		for (int i = 0; i < NUM_CELLS; i++) {
			 sum_voltage += batt->cell_volt[i]; //get sum voltage
		}
		batt->sum_pack_voltage = (uint16_t)(sum_voltage / 100);
	}


