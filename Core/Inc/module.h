#ifndef INC_MODULE_H_
#define INC_MODULE_H_

#include <stdint.h>

#include "6811.h"

#define MD_FAST 1		//27kHz
#define MD_NORMAL 2		//7kHz
#define MD_FILTERED 3	//26Hz
#define FAST_DELAY 2
#define NORMAL_DELAY 4
#define FILTERD_DELAY 202
#define CELL_CH_ALL 0
#define DCP_DISABLED 0

typedef struct batteryModule {
	uint16_t cell_volt[NUM_CELLS];
	uint16_t cell_temp[NUM_THERM_TOTAL];
	uint16_t cell_temp_8bits[NUM_THERM_TOTAL];
	uint16_t average_volt[NUM_DEVICES];
	uint16_t average_temp[NUM_DEVICES];
	uint16_t standerd_diviation;
	uint16_t cell_volt_lowest;
	uint16_t cell_volt_highest;
	uint16_t cell_temp_lowest;
	uint16_t cell_temp_highest;
	uint32_t pack_voltage;
	uint16_t read_auxreg[NUM_AUXES];
    uint16_t soc;
    uint32_t current;
}batteryModule;

void Read_Volt(uint16_t *read_volt);
void Get_Actual_Temps(uint8_t dev_idx, uint8_t tempindex, uint16_t *actual_temp,
		uint16_t data);
void Read_Temp(uint8_t tempindex, uint16_t *read_temp, uint16_t *read_auxreg);

#endif /* INC_MODULE_H_ */
