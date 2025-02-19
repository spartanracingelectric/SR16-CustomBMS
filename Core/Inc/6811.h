
#ifndef INC_6811_H_
#define INC_6811_H_

#include <stdint.h>
#include <stdlib.h>
#include "spi.h"
#include "string.h"

#define LTC_CMD_RDCVA 0x0004
#define LTC_CMD_RDCVB 0x0006
#define LTC_CMD_RDCVC 0x0008
#define LTC_CMD_RDCVD 0x000A

#define LTC_CMD_RDAUXA 0x000C
#define LTC_CMD_RDAUXB 0x000E

#define LTC_SPI_TX_BIT_OFFSET 0	// Num bits to shift RX status code
#define LTC_SPI_RX_BIT_OFFSET 4	// Num bits to shift RX status code
#define REG_LEN 8// number of bytes in the register + 2 bytes for the PEC
#define LTC_SERIES_GROUPS_PER_RDCV 3 // Number of cell voltage groups per 8 byte register
#define LTC_SERIES_GROUPS_PER_RDAUX 3
#define NUM_AUX_SERIES_GROUPS 6 // Number of series groups

#define NUM_DEVICES				8	//1 slave board
#define NUM_CELL_SERIES_GROUP	12	//1 slave board
#define NUM_CELLS				NUM_DEVICES*NUM_CELL_SERIES_GROUP	//multiple slave board
#define NUM_THERM_PER_MOD		12
#define NUM_THERM_TOTAL			NUM_DEVICES*NUM_THERM_PER_MOD
#define NUM_AUX_GROUP			6
#define NUM_AUXES				NUM_DEVICES*NUM_AUX_GROUP
#define CYCLETIME_CAP			100 //100ms update delay
#define LED_HEARTBEAT_DELAY_MS	50  //10ms update delay
#define BALANCE 				0 	//FALSE
#define MAX_CELL_CAPACITY 		3000
#define MAX_BATTERY_CAPACITY 	NUM_DEVICES* MAX_CELL_CAPACITY

typedef enum {
	LTC_SPI_OK = 0x00U, //0b00000000
	LTC_SPI_TX_ERROR = 0x02U, //0b00000010
	LTC_SPI_TX_BUSY = 0x04U, //0b00000100
	LTC_SPI_TX_TIMEOUT = 0x08U, //0b00001000
	LTC_SPI_RX_ERROR = 0x20U, //0b00100000
	LTC_SPI_RX_BUSY = 0x40U, //0b01000000
	LTC_SPI_RX_TIMEOUT = 0x80U	 //0b10000000
} LTC_SPI_StatusTypeDef;

extern uint8_t wrpwm_buffer[4 + (8 * NUM_DEVICES)];
extern uint8_t wrcfg_buffer[4 + (8 * NUM_DEVICES)];
extern uint8_t wrcomm_buffer[4 + (8 * NUM_DEVICES)];

void Wakeup_Idle(void);

void Wakeup_Sleep(void);

LTC_SPI_StatusTypeDef Read_Cell_Volt(uint16_t *read_voltages);

/* write to PWM register to control balancing functionality */
void LTC6811_WRPWM(uint8_t total_ic, uint8_t pwm);

void LTC6811_WRCFG(uint8_t total_ic, //The number of ICs being written to
		uint8_t config[][6] //A two dimensional array of the configuration data that will be written
		);

void LTC_WRCOMM(uint8_t total_ic, //The number of ICs being written to
		uint8_t comm[6] //A two dimensional array of the comm data that will be written
		);

void LTC_STCOMM(uint8_t len);

LTC_SPI_StatusTypeDef Read_Cell_Temps(uint16_t *read_auxiliary);

void LTC_ADCV(uint8_t MD, //ADC Mode
		uint8_t DCP, //Discharge Permit
		uint8_t CH //Cell Channels to be measured
		);

void LTC_ADAX(uint8_t MD, //ADC Mode
		uint8_t CHG //GPIO Channels to be measured)
		);

int32_t LTC_POLLADC();

int Calc_Pack_Voltage(uint16_t *read_voltages);

uint16_t LTC_Pec15_Calc(uint8_t len, //Number of bytes that will be used to calculate a PEC
		uint8_t *data //Array of data that will be used to calculate a PEC
		);

#endif /* INC_6811_H_ */
