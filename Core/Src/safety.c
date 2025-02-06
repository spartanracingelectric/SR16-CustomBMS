#include "safety.h"
#include "main.h"
#include "stdio.h"
// ! Fault Thresholds

// Refer to TODO on Line 61
#define PACK_HIGH_VOLT_FAULT	    4100000
#define PACK_LOW_VOLT_FAULT         2880000
#define CELL_HIGH_VOLT_FAULT	    42000
#define CELL_LOW_VOLT_FAULT		    25000
#define CELL_VOLT_IMBALANCE_FAULT   2000 //0.1 V
#define CELL_HIGH_TEMP_FAULT		60

// ! Warnings Thresholds
#define PACK_HIGH_VOLT_WARNING	    4085000
#define PACK_LOW_VOLT_WARNING       3000000
#define CELL_HIGH_VOLT_WARNING	    40000
#define CELL_LOW_VOLT_WARNING	    27000
#define CELL_VOLT_IMBALANCE_WARNING	1000 //0.05 V
#define CELL_HIGH_TEMP_WARNING		55
#define CELL_LOW_TEMP_WARNING		0

#define FAULT_LOCK_MARGIN_HIGH_VOLT 100			//10 mV
#define FAULT_LOCK_MARGIN_LOW_VOLT 	1000		//100 mV
#define FAULT_LOCK_MARGIN_IMBALANCE 1000		//100 mV
#define FAULT_LOCK_MARGIN_HIGH_TEMP 10			//10 ℃

#define WARNING_BIT_HIGH_PACK_VOLT 	(1 << 7)	// 0b10000000 (Bit position 7)
#define WARNING_BIT_LOW_PACK_VOLT  	(1 << 6)	// 0b01000000 (Bit position 6)
#define WARNING_BIT_HIGH_VOLT    	(1 << 5)  	// 0b00010000 (Bit position 5)
#define WARNING_BIT_LOW_VOLT     	(1 << 4)  	// 0b00001000 (Bit position 4)
#define WARNING_BIT_IMBALANCE       (1 << 3)  	// 0b00000100 (Bit position 3)
#define WARNING_BIT_HIGH_TEMP 		(1 << 2) 	// 0b00000010 (Bit position 2)
#define WARNING_BIT_SLAVE_VOLT 		(1 << 1) 	// 0b00000001 (Bit position 1)

#define FAULT_BIT_HIGH_PACK_VOLT 	(1 << 7)	// 0b10000000 (Bit position 7)
#define FAULT_BIT_LOW_PACK_VOLT  	(1 << 6)	// 0b01000000 (Bit position 6)
#define FAULT_BIT_HIGH_VOLT    		(1 << 5)  	// 0b00010000 (Bit position 5)
#define FAULT_BIT_LOW_VOLT     		(1 << 4)  	// 0b00001000 (Bit position 4)
#define FAULT_BIT_IMBALANCE       	(1 << 3)  	// 0b00000100 (Bit position 3)
#define FAULT_BIT_HIGH_TEMP 		(1 << 2) 	// 0b00000010 (Bit position 2)

#define SLAVE_VOLT_WARNING_MARGIN 	100			//10 mV


void Cell_Voltage_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings, uint8_t *states,
                        uint8_t *high_volt_fault_lock,  uint8_t *low_volt_hysteresis, uint8_t *low_volt_fault_lock,
                        uint8_t *cell_imbalance_hysteresis){

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
			*high_volt_fault_lock = 1;
			*fault |= FAULT_BIT_HIGH_VOLT;
			HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_SET);
//			printf("high voltage fault signal on\n");
		}
		//reset high cell volt fault
		else if (batt->cell_volt_highest < (CELL_HIGH_VOLT_FAULT - FAULT_LOCK_MARGIN_HIGH_VOLT) && *high_volt_fault_lock == 1){
			*high_volt_fault_lock = 0;
			*warnings &= ~WARNING_BIT_HIGH_VOLT;
			*fault &= ~FAULT_BIT_HIGH_VOLT;
			HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_RESET);
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
		if(batt->cell_volt_lowest <= CELL_LOW_VOLT_FAULT && *low_volt_hysteresis == 1){
			*fault |= FAULT_BIT_LOW_VOLT;
			HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_SET);
		}
		//reset low cell volt fault
		else if(batt->cell_volt_lowest > (CELL_LOW_VOLT_FAULT + FAULT_LOCK_MARGIN_LOW_VOLT) && *low_volt_hysteresis == 1){
			*low_volt_hysteresis = 0;
			*warnings &= ~WARNING_BIT_LOW_VOLT;
			*fault &= ~FAULT_BIT_LOW_VOLT;
			HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_RESET);
		}
		//low cell volt fault(hysteresis)
		if (batt->cell_volt_lowest <= CELL_LOW_VOLT_FAULT) {
			*low_volt_hysteresis = 1;//use hysteresis and spend 2 cycle to fault
		}
		else if (batt->cell_volt_lowest > CELL_LOW_VOLT_FAULT) {
			*low_volt_hysteresis = 0;//use hysteresis and spend 2 cycle to fault
		}




		//cell volt imbalance warning
//		if ((batt->cell_volt_highest - batt->cell_volt_lowest) >= CELL_VOLT_IMBALANCE_WARNING) {
//			*warnings |= WARNING_BIT_IMBALANCE;
//		}
//		//cell volt imbalance fault
//		if (((batt->cell_volt_highest - batt->cell_volt_lowest) >= CELL_VOLT_IMBALANCE_FAULT) && (*cell_imbalance_hysteresis == 1)) {
//			*fault |= FAULT_BIT_IMBALANCE;
//			HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_SET);
//		}
//		//reset cell volt imbalance fault
//		else if (((batt->cell_volt_highest - batt->cell_volt_lowest) < FAULT_LOCK_MARGIN_IMBALANCE) && (*cell_imbalance_hysteresis == 1)){
//			*cell_imbalance_hysteresis = 0;
//			*warnings &= ~WARNING_BIT_IMBALANCE;
//			*fault &= ~FAULT_BIT_IMBALANCE;
//			HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_RESET);
//		}
//		//cell volt imbalance fault(hysteresis)
//		if ((batt->cell_volt_highest - batt->cell_volt_lowest) >= CELL_VOLT_IMBALANCE_FAULT) {
//			*cell_imbalance_hysteresis = 1;
//		}
//		else if ((batt->cell_volt_highest - batt->cell_volt_lowest) < CELL_VOLT_IMBALANCE_FAULT) {
//			*cell_imbalance_hysteresis = 0;
//		}
//		if (BALANCE) {
//			*states |= 0b10000000;
//		}
	}
}

void Cell_Temperature_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings, uint8_t *high_temp_hysteresis) {
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
		if (batt->cell_temp_highest >= CELL_HIGH_TEMP_FAULT && *high_temp_hysteresis == 1) {
			*fault |= FAULT_BIT_HIGH_TEMP;
			HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_SET);
		}
		//reset highest cell temp fault
		else if (batt->cell_temp_highest < (CELL_HIGH_TEMP_FAULT - FAULT_LOCK_MARGIN_HIGH_TEMP) && *high_temp_hysteresis == 1){
			*warnings &= ~WARNING_BIT_HIGH_TEMP;
			*fault &= ~FAULT_BIT_HIGH_TEMP;
			HAL_GPIO_WritePin(MCU_SHUTDOWN_SIGNAL_GPIO_Port, MCU_SHUTDOWN_SIGNAL_Pin, GPIO_PIN_RESET);
		}
		//highest cell temp fault(hysteresis)
		if (batt->cell_temp_highest >= CELL_HIGH_TEMP_FAULT) {
			*high_temp_hysteresis = 1;
		}
		else if (batt->cell_temp_highest < CELL_HIGH_TEMP_FAULT) {
			*high_temp_hysteresis = 0;
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
