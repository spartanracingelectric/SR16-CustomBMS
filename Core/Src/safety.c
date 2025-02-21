#include "safety.h"
#include "main.h"
#include "stdio.h"
#include "gpio.h"
// ! Fault Thresholds

uint8_t high_volt_fault_lock = 0;
uint8_t low_volt_hysteresis = 0;
uint8_t low_volt_fault_lock = 0;
uint8_t cell_imbalance_hysteresis = 0;
uint8_t high_temp_hysteresis = 0;

void Cell_Voltage_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings){

	batt->cell_volt_highest = batt->cell_volt[0];
	batt->cell_volt_lowest = batt->cell_volt[0];

	for (int i = 1; i < NUM_CELLS; i++) {
		//find highest volt
		if (batt->cell_volt[i] > batt->cell_volt_highest) {
			batt->cell_volt_highest = batt->cell_volt[i];
//			printf("high voltage fault: %d\n", batt->cell_volt_highest);
		}
		//high cell volt warning
		if (batt->cell_volt_highest >= CELL_HIGH_VOLT_WARNING && batt->cell_volt_highest < CELL_HIGH_VOLT_FAULT) {
			*warnings |= WARNING_BIT_HIGH_VOLT;
		}
		//high cell volt fault
		if ((batt->cell_volt_highest >= CELL_HIGH_VOLT_FAULT) /*&& ((*high_volt_hysteresis) > 0)*/) {
			high_volt_fault_lock = 1;
			*fault |= FAULT_BIT_HIGH_VOLT;
			SendFaultSignal();
//			printf("high voltage fault signal on\n");
		}
		//reset high cell volt fault
		else if (batt->cell_volt_highest < (CELL_HIGH_VOLT_FAULT - FAULT_LOCK_MARGIN_HIGH_VOLT) && high_volt_fault_lock == 1){
			high_volt_fault_lock = 0;
			*warnings &= ~WARNING_BIT_HIGH_VOLT;
			*fault &= ~FAULT_BIT_HIGH_VOLT;
			ClearFaultSignal();
		}

		//find lowest volt
		if (batt->cell_volt[i] < batt->cell_volt_lowest) {
			batt->cell_volt_lowest = batt->cell_volt[i];
		}
		//low cell volt warning
		if (batt->cell_volt_lowest <= CELL_LOW_VOLT_WARNING) {
			*warnings |= WARNING_BIT_LOW_VOLT;
		}
		//low cell volt fault
		if(batt->cell_volt_lowest <= CELL_LOW_VOLT_FAULT && * low_volt_hysteresis > 3){
			*fault |= FAULT_BIT_LOW_VOLT;
			SendFaultSignal();
		}
		//reset low cell volt fault
		else if(batt->cell_volt_lowest > (CELL_LOW_VOLT_FAULT + FAULT_LOCK_MARGIN_LOW_VOLT) && *low_volt_hysteresis > 0){
			*low_volt_hysteresis = 0;
			*warnings &= ~WARNING_BIT_LOW_VOLT;
			*fault &= ~FAULT_BIT_LOW_VOLT;
			ClearFaultSignal();
		}
		//low cell volt fault(hysteresis)
		if (batt->cell_volt_lowest <= CELL_LOW_VOLT_FAULT) {
			*low_volt_hysteresis++;//use hysteresis and spend 2 cycle to fault
		}
		else if (batt->cell_volt_lowest > (CELL_LOW_VOLT_FAULT + FAULT_LOCK_MARGIN_LOW_VOLT)) {
			*low_volt_hysteresis = 0;//use hysteresis and spend 2 cycle to fault
		}
	}
}

void Cell_Blance_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings) {
	batt->cell_difference = (uint8_t)(batt->cell_volt_highest - batt->cell_volt_lowest);
//cell volt imbalance warning
	if (batt->cell_difference >= CELL_VOLT_IMBALANCE_WARNING) {
		*warnings |= WARNING_BIT_IMBALANCE;
	}
	//cell volt imbalance fault
	if ((batt->cell_difference >= CELL_VOLT_IMBALANCE_FAULT) && (cell_imbalance_hysteresis == 1)) {
		*fault |= FAULT_BIT_IMBALANCE;
		SendFaultSignal();
	}
	//reset cell volt imbalance fault
	else if ((batt->cell_difference < FAULT_LOCK_MARGIN_IMBALANCE) && (cell_imbalance_hysteresis == 1)){
		cell_imbalance_hysteresis = 0;
		*warnings &= ~WARNING_BIT_IMBALANCE;
		*fault &= ~FAULT_BIT_IMBALANCE;
		ClearFaultSignal();
	}
	//cell volt imbalance fault(hysteresis)
	if (batt->cell_difference >= CELL_VOLT_IMBALANCE_FAULT) {
		cell_imbalance_hysteresis = 1;
	}
	else if (batt->cell_difference < CELL_VOLT_IMBALANCE_FAULT) {
		cell_imbalance_hysteresis = 0;
	}
}

void Cell_Temperature_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings) {
	batt->cell_temp_highest = batt->cell_temp[0];
	batt->cell_temp_lowest = batt->cell_temp[0];

	for (int i = 0; i < NUM_THERM_TOTAL; i++) {
		//find highest temp
		if (batt->cell_temp_highest < batt->cell_temp[i]) {
			batt->cell_temp_highest = batt->cell_temp[i];
		}
		//highest cell temp warning
		if (batt->cell_temp_highest >= CELL_HIGH_TEMP_WARNING && batt->cell_temp_highest < CELL_HIGH_TEMP_FAULT) {
			*warnings |= WARNING_BIT_HIGH_TEMP;
		}
		//highest cell temp fault
<<<<<<< HEAD
		if (batt->cell_temp_highest >= CELL_HIGH_TEMP_FAULT && *high_temp_hysteresis > 3) {
=======
		if (batt->cell_temp_highest >= CELL_HIGH_TEMP_FAULT && high_temp_hysteresis == 1) {
>>>>>>> 41d52a4 (added can sending function for balance status)
			*fault |= FAULT_BIT_HIGH_TEMP;
			SendFaultSignal();
		}
		//reset highest cell temp fault
<<<<<<< HEAD
		else if (batt->cell_temp_highest < (CELL_HIGH_TEMP_FAULT - FAULT_LOCK_MARGIN_HIGH_TEMP) && *high_temp_hysteresis > 0){
=======
		else if (batt->cell_temp_highest < (CELL_HIGH_TEMP_FAULT - FAULT_LOCK_MARGIN_HIGH_TEMP) && high_temp_hysteresis == 1){
>>>>>>> 41d52a4 (added can sending function for balance status)
			*warnings &= ~WARNING_BIT_HIGH_TEMP;
			*fault &= ~FAULT_BIT_HIGH_TEMP;
			ClearFaultSignal();
		}
		//highest cell temp fault(hysteresis)
		if (batt->cell_temp_highest >= CELL_HIGH_TEMP_FAULT) {
<<<<<<< HEAD
			*high_temp_hysteresis++;
		}
		else if (batt->cell_temp_highest < (CELL_HIGH_TEMP_FAULT - FAULT_LOCK_MARGIN_HIGH_TEMP)) {
			*high_temp_hysteresis = 0;
=======
			high_temp_hysteresis = 1;
		}
		else if (batt->cell_temp_highest < CELL_HIGH_TEMP_FAULT) {
			high_temp_hysteresis = 0;
>>>>>>> 41d52a4 (added can sending function for balance status)
		}
	}
}

//void High_Voltage_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings){
//	uint32_t sum_voltage = 0;
//
//	for (int i = 0; i < NUM_CELLS; i++) {
//		 sum_voltage += (uint32_t)batt->cell_volt[i]; //get sum voltage
//	}
//	if ((sum_voltage - batt->pack_voltage) >= FAULT_LOCK_MARGIN_LOW_VOLT){
//		*warnings |= WARNING_BIT_SLAVE_VOLT;
//	}
//	if (batt->pack_voltage >= PACK_HIGH_VOLT_WARNING) {
//		*warnings |= WARNING_BIT_HIGH_PACK_VOLT;
//	}
//	if (batt->pack_voltage <= PACK_LOW_VOLT_WARNING) {
//		*warnings |= WARNING_BIT_LOW_PACK_VOLT;
//	}
//	if (batt->pack_voltage >= PACK_HIGH_VOLT_FAULT) {
//		*fault |= FAULT_BIT_HIGH_PACK_VOLT;
//		HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_SET);
//	}
//	else{
//		*fault &= ~FAULT_BIT_HIGH_PACK_VOLT;
//	}
//	if (batt->pack_voltage <= PACK_LOW_VOLT_FAULT) {
//		*fault |= FAULT_BIT_LOW_PACK_VOLT;
//		HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_SET);
//	}
//	else{
//		*fault &= ~FAULT_BIT_LOW_PACK_VOLT;
//	}
//}


//void Module_Voltage_Averages(struct batteryModule *batt) {
//    for (int i = 0; i < NUM_CELLS; i += NUM_CELL_SERIES_GROUP) {
//        uint16_t volt_sum = 0;
//
//        for (int j = i; j < i + NUM_CELL_SERIES_GROUP && j < NUM_CELLS; j++) {
//            volt_sum += batt->cell_volt[j];
//        }
//
//        uint16_t average = volt_sum / NUM_CELL_SERIES_GROUP;
//
//        batt->average_volt[i / NUM_CELL_SERIES_GROUP] = average;
//    }
//}
//
//
//void Module_Temperature_Averages(struct batteryModule *batt) {
//    for (int i = 0; i < NUM_THERM_TOTAL; i += NUM_THERM_PER_MOD) {
//        uint16_t temp_sum = 0;
//
//        for (int j = i; j < i + NUM_THERM_PER_MOD && j < NUM_THERM_TOTAL; j++) {
//            temp_sum += batt->cell_temp[j];
//        }
//
//        uint16_t average = temp_sum / NUM_THERM_PER_MOD;
//
//        batt->average_temp[i / NUM_THERM_PER_MOD] = average;
//    }
//}
