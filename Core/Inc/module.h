#ifndef INC_MODULE_H_
#define INC_MODULE_H_

#include "main.h"

#include <stdint.h>

#define MD_FAST 1		//27kHz
#define MD_NORMAL 2		//7kHz
#define MD_FILTERED 3	//26Hz
#define FAST_DELAY 2
#define NORMAL_DELAY 4
#define FILTERD_DELAY 202
#define CELL_CH_ALL 0
#define DCP_DISABLED 0

#define SENSOR_MIN_TEMP -40.0f
#define SENSOR_MAX_TEMP 125.0f

#define LTC6811_Vdd 51450.0f


void Read_Volt(uint16_t *read_volt);
void Get_Actual_Temps(uint8_t dev_idx, uint8_t tempindex, uint16_t *actual_temp,
		uint16_t data);

void Read_Temp(uint8_t tempindex, uint16_t *read_temp, uint16_t *read_auxreg);

void Convert_Analog_To_Pressure(uint8_t dev_idx, uint16_t *pressure, uint16_t adc_data);

void Atmos_Temp_To_Celsius(uint8_t dev_idx, uint16_t *pressure, uint16_t adc_data);

void ADC_To_Humidity(uint8_t dev_idx, uint16_t *humidity, uint16_t adc_data);

void Read_Pressure(batteryModule *batt);

void Read_Atmos_Temp(batteryModule *batt);

void Read_Humidity(batteryModule *batt);

void Get_Dew_Point(batteryModule *batt);

#endif /* INC_MODULE_H_ */
