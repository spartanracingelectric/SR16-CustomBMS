#ifndef INC_MODULE_H_
#define INC_MODULE_H_

#include "main.h"

#define MD_FAST 1		//27kHz
#define MD_NORMAL 2		//7kHz
#define MD_FILTERED 3	//26Hz
#define FAST_DELAY 2
#define NORMAL_DELAY 4
#define FILTERD_DELAY 202
#define CELL_CH_ALL 0
#define DCP_DISABLED 0

void Thermistor_To_Celsius(uint8_t dev_idx, uint8_t tempindex, uint16_t *actual_temp,
		uint16_t data);

void Convert_Analog_To_Pressure(uint8_t dev_idx, uint16_t *pressure, uint16_t adc_data);

void Atmos_Temp_To_Celsius(uint8_t dev_idx, uint16_t *pressure, uint16_t adc_data);

void ADC_To_Humidity(uint8_t dev_idx, uint16_t *humidity, uint16_t adc_data);

void Read_Volt(uint16_t *read_volt);

void Read_Thermistor(uint8_t tempindex, uint16_t *read_temp, uint16_t *read_auxreg);

void Read_Pressure(uint16_t *read_pressure, uint16_t *read_auxreg);

void Read_Atmos_Temp(uint16_t *read_pressure, uint16_t *read_auxreg);

void Read_Humidity(uint16_t *read_pressure, uint16_t *read_auxreg);

#endif /* INC_MODULE_H_ */
