#ifndef INC_SAFETY_H_
#define INC_SAFETY_H_

#include "main.h"

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

void Cell_Voltage_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings);

void Cell_Balance_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings);

void Cell_Temperature_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings);

void High_Voltage_Fault(struct batteryModule *batt, uint8_t *fault, uint8_t *warnings);

void Module_Voltage_Averages(struct batteryModule *batt);

void Module_Temperature_Averages(struct batteryModule *batt);

#endif /* INC_SAFETY_H_ */
