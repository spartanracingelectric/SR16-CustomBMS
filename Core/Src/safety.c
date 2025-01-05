#include "safety.h"

// ! Fault Thresholds

// Refer to TODO on Line 61
//#define PACK_HIGH_VOLT_FAULT	    410000
//#define PACK_LOW_VOLT_FAULT         288000

#define CELL_HIGH_VOLT_FAULT	    42000
#define CELL_LOW_VOLT_FAULT		    25000

#define CELL_HIGH_TEMP_FAULT		60

// ! Warnings Thresholds
//#define PACK_HIGH_VOLT_WARNING	    408500
//#define PACK_LOW_VOLT_WARNING       300000

#define CELL_HIGH_VOLT_WARNING	    40000
#define CELL_LOW_VOLT_WARNING	    27000

#define CELL_HIGH_TEMP_WARNING		55
#define CELL_LOW_TEMP_WARNING		0

#define CELL_VOLT_IMBALANCE_FAULT   2000 //0.1 V
#define CELL_VOLT_IMBALANCE_WARNING	1000 //0.05 V

void Cell_Summary(struct batteryModule *batt) {
	batt->cell_volt_highest = batt->cell_volt[0];
	batt->cell_volt_lowest = batt->cell_volt[0];
	batt->cell_temp_highest = batt->cell_temp[0];
	batt->cell_temp_lowest = batt->cell_temp[0];
	batt->pack_voltage = (uint32_t) batt->cell_volt[0];

	for (int i = 1; i < NUM_CELLS; i++) {

		if (batt->cell_volt[i] > batt->cell_volt_highest) {
			batt->cell_volt_highest = batt->cell_volt[i];
		}

		if (batt->cell_volt[i] < batt->cell_volt_lowest) {
			batt->cell_volt_lowest = batt->cell_volt[i];
		}

		batt->pack_voltage += (uint32_t) batt->cell_volt[i];
	}

	for (int i = 0; i < NUM_THERM_TOTAL; i++) {
		if (batt->cell_temp_highest < batt->cell_temp[i]) {
			batt->cell_temp_highest = batt->cell_temp[i];
		}

		if (batt->cell_temp_lowest > batt->cell_temp[i]) {
			batt->cell_temp_lowest = batt->cell_temp[i];
		}
	}

}

void Fault_Warning_State(struct batteryModule *batt, uint8_t *fault,
		uint8_t *warnings, uint8_t *states, uint8_t *low_volt_hysteresis,
		uint8_t *high_volt_hysteresis, uint8_t *cell_imbalance_hysteresis) {
//	TODO: 2024-2025 Season: Used Sum of Cell Voltages, thus just used MCU DC Bus Voltage, future addition could be nice
//	if (batt->pack_voltage >= PACK_HIGH_VOLT_FAULT) {
//		*fault |= 0b10000000;
//	}
//
//	if (batt->pack_voltage <= PACK_LOW_VOLT_FAULT) {
//		*fault |= 0b01000000;
//	}

	//low cell volt fault
	if ((batt->cell_volt_lowest <= CELL_LOW_VOLT_FAULT)
			&& ((*low_volt_hysteresis) > 0)) {
		*fault |= 0b00100000;
	} else if (batt->cell_volt_lowest <= CELL_LOW_VOLT_FAULT) {
		*low_volt_hysteresis = 1;
	} else {
		*low_volt_hysteresis = 0;
	}
	//end of low cell volt fault

	//high cell volt fault
	if ((batt->cell_volt_highest >= CELL_HIGH_VOLT_FAULT)
			&& ((*high_volt_hysteresis) > 0)) {
		*fault |= 0b00010000;
	} else if (batt->cell_volt_highest >= CELL_HIGH_VOLT_FAULT) {
		*high_volt_hysteresis = 1;
	} else {
		*high_volt_hysteresis = 0;
	}
	//end of high cell volt fault

	//highest cell temp fault
	if (batt->cell_temp_highest >= CELL_HIGH_TEMP_FAULT) {
		*fault |= 0b00001000;
	}

	//cell volt imbalance fault
	if (((batt->cell_volt_highest - batt->cell_volt_lowest)
			>= CELL_VOLT_IMBALANCE_FAULT)
			&& ((*cell_imbalance_hysteresis) > 0)) {
		*fault |= 0b00000100;
	} else if ((batt->cell_volt_highest - batt->cell_volt_lowest)
			>= CELL_VOLT_IMBALANCE_FAULT) {
		*cell_imbalance_hysteresis = 1;
	} else {
		*cell_imbalance_hysteresis = 0;
	}
	//end of cell volt imbalance fault

//	if (batt->pack_voltage >= PACK_HIGH_VOLT_WARNING) {
//		*warnings |= 0b10000000;
//	}
//
//	if (batt->pack_voltage <= PACK_LOW_VOLT_WARNING) {
//		*warnings |= 0b01000000;
//	}

	if (batt->cell_volt_lowest <= CELL_LOW_VOLT_WARNING) {
		*warnings |= 0b00100000;
	}

	if (batt->cell_volt_highest >= CELL_HIGH_VOLT_WARNING) {
		*warnings |= 0b00010000;
	}

	if (batt->cell_temp_highest >= CELL_HIGH_TEMP_WARNING) {
		*warnings |= 0b00001000;
	}

	if (batt->cell_temp_lowest <= CELL_LOW_TEMP_WARNING) {
		*warnings |= 0b00000100;
	}

	if ((batt->cell_volt_highest - batt->cell_volt_lowest)
			>= CELL_VOLT_IMBALANCE_WARNING) {
		*warnings |= 0b00000010;
	}

	if (BALANCE) {
		*states |= 0b10000000;
	}
}

void Module_Averages(struct batteryModule *batt) {

	for (int i = 0; i < NUM_CELLS; i += 12) {
		uint16_t temp_sum = 0;

		for (int j = 0; j < i + 12; j++) {
			temp_sum += batt->cell_volt[i];
		}

		uint16_t average = temp_sum / NUM_CELL_SERIES_GROUP;

		switch (i / 12) {
		case 0:
			batt->module_averages[i] = average;
			break;
		case 1:
			batt->module_averages[i] = average;
			break;
		case 2:
			batt->module_averages[i] = average;
			break;
		case 3:
			batt->module_averages[i] = average;
			break;
		case 4:
			batt->module_averages[i] = average;
			break;
		case 5:
			batt->module_averages[i] = average;
			break;
		case 6:
			batt->module_averages[i] = average;
			break;
		case 7:
			batt->module_averages[i] = average;
			break;
		}
	}
}
