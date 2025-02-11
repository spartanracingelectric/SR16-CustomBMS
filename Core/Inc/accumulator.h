#ifndef INC_ACCUMULATOR_H_
#define INC_ACCUMULATOR_H_

#include <stdint.h>

#define MD_FAST 1      // 27kHz
#define MD_NORMAL 2    // 7kHz
#define MD_FILTERED 3  // 26Hz
#define FAST_DELAY 2
#define NORMAL_DELAY 4
#define FILTERD_DELAY 202
#define CELL_CH_ALL 0
#define DCP_DISABLED 0

#define NUM_DEVICES 8  // 1 slave board
#define NUM_PARALLELS 8
#define NUM_CELL_SERIES_GROUP 12                         // 1 slave board
#define NUM_CELLS (NUM_DEVICES * NUM_CELL_SERIES_GROUP)  // multiple slave board
#define NUM_THERM_PER_MOD 12
#define NUM_THERM_TOTAL (NUM_DEVICES * NUM_THERM_PER_MOD)
#define NUM_AUX_GROUP 6
#define NUM_AUXES (NUM_DEVICES * NUM_AUX_GROUP)
#define LTC_DELAY 1000             // 1s update delay
#define LED_HEARTBEAT_DELAY_MS 50  // 10ms update delay
#define BALANCE 0                  // FALSE
#define MAX_CELL_CAPACITY 3000
#define MAX_ACCUMULATOR_CAPACITY (NUM_PARALLELS * MAX_CELL_CAPACITY)

typedef struct Accumulator {
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
} Accumulator;

void Accumulator_readVoltages(uint16_t* read_volt);
void Accumulator_readTemperatures(uint8_t tempindex, uint16_t* read_temp,
                                  uint16_t* read_auxreg);

#endif
