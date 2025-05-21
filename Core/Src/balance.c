#include "balance.h"
#include "6811.h"
#include "can.h"
#include <stdio.h>
#include "usart.h"

//DEFAULT VALUES THAT ARE SET IN CONFIG REGISTERS
//static int GPIO[5] = { 1, 1, 1, 1, 1 };
//static int REFON = 0;
//static int DTEN = 1; (READ ONLY BIT, we dont change it)
//static int ADCOPT = 0;
//static uint8_t VUV = 0x00;
//static uint8_t VOV_and_VUV = 0x00;
//static uint8_t VOV = 0x00;
//static int DCTO[4] = { 1, 1, 1, 1 };

static uint8_t config[8][6] = { { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 }, { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 },
								{ 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 }, { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 },
								{ 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 }, { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 },
								{ 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 }, { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 } };

static uint8_t defaultConfig[8][6] = {{ 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 }, { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 },
									  { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 }, { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 },
									  { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 }, { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 },
									  { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 }, { 0xF8, 0x00, 0x00, 0x00, 0x00, 0x20 } };

void Balance_init(uint16_t *balanceStatus){
	balance = 0;
	balance_finish = 0;
	Balance_reset(balanceStatus);
	Wakeup_Sleep();
	LTC_writeCFG(NUM_DEVICES, defaultConfig);
}



void Start_Balance(uint16_t *read_volt, uint16_t lowest, uint16_t *balanceStatus) {
//	printf("balance enable is %d\n", balance);
	if(balance > 0){
		Discharge_Algo(read_volt, lowest , balanceStatus);
		Wakeup_Sleep();
		LTC_writeCFG(NUM_DEVICES, config);
	}
	else{
		return;
	}
}

void End_Balance(uint16_t *balanceStatus) {
	if(balance_finish == 1){
		Balance_reset(balanceStatus);
		Wakeup_Sleep();
		LTC_writeCFG(NUM_DEVICES, defaultConfig);
		balance_finish = 0;
	}
	else{
		return;
	}
}

/**
 * perform balance
 * 
 * @param read_volt array containing cells volts. 
 * @param length count of readings. 
 * @param lowest read_volt's lowest cell reading
 */
void Discharge_Algo(uint16_t *read_volt, uint16_t lowest, uint16_t *balanceStatus) {
	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
		// check if each cell is close within 0.005V of the lowest cell.
		uint8_t DCC[12];
		for (uint8_t cell_idx = 0; cell_idx < NUM_CELL_SERIES_GROUP; cell_idx++) {
			if (read_volt[dev_idx * NUM_CELL_SERIES_GROUP + cell_idx] - lowest > BALANCE_THRESHOLD) {
				DCC[cell_idx] = 1;
				balanceStatus[dev_idx] |= (1 << cell_idx);
			} else {
				DCC[cell_idx] = 0;
				balanceStatus[dev_idx] &= ~(1 << cell_idx); //set the bit to 0
			}
		}
		Set_Cfg(dev_idx, (uint8_t*) DCC);
	}
}

void Balance_reset(uint16_t *balanceStatus) {
	uint8_t DCC[12] = {0};  //reset all DCC to 0
	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
		balanceStatus[dev_idx] = 0;
//		printf("balanceStaus[%d]: %d\n", dev_idx, balanceStatus[dev_idx]);

		Set_Cfg(dev_idx, (uint8_t*) DCC);
	}
}

/**
 * setting configuration registers
 *
 * @param device index
 * @param array of DCC bits
 */
void Set_Cfg(uint8_t dev_idx, uint8_t *DCC) {
	for (uint8_t cell_idx = 0; cell_idx < NUM_CELL_SERIES_GROUP; cell_idx++) {
		if (DCC[cell_idx]) {
			if (cell_idx < 8) {
				config[dev_idx][4] |= (1 << cell_idx);
			} else if (cell_idx >= 8) {
				config[dev_idx][5] |= (1 << (cell_idx - 8));
			}
		} else {
			if (cell_idx < 8) {
				config[dev_idx][4] &= (~(1 << cell_idx));
			} else {
				config[dev_idx][5] &= (~(1 << (cell_idx - 8)));
			}
		}
	}
}

