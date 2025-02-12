#include "module.h"
#include <math.h>
#include "6811.h"
#include <stdio.h>

#define ntcNominal 10000.0f
#define ntcSeriesResistance 10000.0f
#define ntcBetaFactor 3435.0f
#define ntcNominalTemp 25.0f

static const float invNominalTemp = 1.0f / (ntcNominalTemp + 273.15f);
static const float invBetaFactor = 1.0f / ntcBetaFactor;

static uint8_t BMS_MUX[][6] = { { 0x69, 0x28, 0x0F, 0xF9, 0x7F, 0xF9 }, { 0x69, 0x28, 0x0F, 0xE9, 0x7F, 0xF9 },
								{ 0x69, 0x28, 0x0F, 0xD9, 0x7F, 0xF9 }, { 0x69, 0x28, 0x0F, 0xC9, 0x7F, 0xF9 },
								{ 0x69, 0x28, 0x0F, 0xB9, 0x7F, 0xF9 }, { 0x69, 0x28, 0x0F, 0xA9, 0x7F, 0xF9 },
								{ 0x69, 0x28, 0x0F, 0x99, 0x7F, 0xF9 }, { 0x69, 0x28, 0x0F, 0x89, 0x7F, 0xF9 },
								{ 0x69, 0x08, 0x0F, 0xF9, 0x7F, 0xF9 }, { 0x69, 0x08, 0x0F, 0xE9, 0x7F, 0xF9 },
								{ 0x69, 0x08, 0x0F, 0xD9, 0x7F, 0xF9 }, { 0x69, 0x08, 0x0F, 0xC9, 0x7F, 0xF9 },
							 	{ 0x69, 0x08, 0x0F, 0xB9, 0x7F, 0xF9 }, { 0x69, 0x08, 0x0F, 0xA9, 0x7F, 0xF9 },
								{ 0x69, 0x08, 0x0F, 0x99, 0x7F, 0xF9 }, { 0x69, 0x08, 0x0F, 0x89, 0x7F, 0xF9 } };

void Thermistor_To_Celsius(uint8_t dev_idx, uint8_t tempindex, uint16_t *actual_temp, uint16_t data) {
    if (data == 0) {
        actual_temp[dev_idx * NUM_THERM_PER_MOD + tempindex] = 999.0f; // error value
        return;
    }

    float scalar = 30000.0f / (float)(data) - 1.0f;
    scalar = ntcSeriesResistance / scalar;

    float steinhart = scalar / ntcNominal;
    steinhart = log(steinhart);
    steinhart *= invBetaFactor;
    steinhart += invNominalTemp;
    steinhart = 1.0f / steinhart;
    steinhart -= 273.15f;

    actual_temp[dev_idx * NUM_THERM_PER_MOD + tempindex] = steinhart;
}

void ADC_To_Pressure(uint8_t dev_idx, uint16_t *pressure, uint16_t adc_data) {
    float voltage = adc_data * (3.0 / 65535.0);  // convert the adc value based on Vref

    float pressure_value = (voltage - 0.5) * (100.0 / 4.0);  //Calculate pressure

    pressure[dev_idx] = (uint16_t)(pressure_value * 10);  // 圧力値を整数に変換
}

void Atmos_Temp_To_Celsius(uint8_t dev_idx, uint16_t *read_atmos_temp, uint16_t adc_data) {
    float voltage = adc_data * (3.0 / 65535.0);  // convert the adc value based on Vref

    float temperature_value = (voltage - 0.5) * (100.0 / 4.0);  //Calculate pressure

    read_atmos_temp[dev_idx] = (uint16_t)(temperature_value * 10);  // 圧力値を整数に変換
}

void Get_AVG_Atmos_Temp(batteryModule *batt, uint8_t dev_idx, uint16_t *read_atmos_temp,){
	float avg_temp = 0;
	for(int i = 0; i < NUM_DEVICES; i++){
		avg_temp += read_atmos_temp[i];
		avg_temp /= NUM_DEVICES;
	}

	batt -> avg_atmos_temp = avg_temp;
}

void ADC_To_Humidity(uint8_t dev_idx, uint16_t *humidity, uint16_t adc_data) {
    float voltage = adc_data * (3.0 / 65535.0);  // convert the adc value based on Vref

    float humidity_value = (-12.5 + 125.0 * (voltage / 3.0));  //Calculate pressure

    humidity[dev_idx] = (uint16_t)(humidity_value * 10);  // 圧力値を整数に変換
}

void Get_AVG_Humidity(batteryModule *batt, uint8_t dev_idx, uint16_t *humidity,){
	float avg_humidity = 0;
	for(int i = 0; i < NUM_DEVICES; i++){
		avg_humidity += humidity[i];
		avg_humidity /= NUM_DEVICES;
	}

	batt -> avg_humidity = avg_humidity;
}


void Read_Volt(uint16_t *read_volt) {
//	printf("volt start\n");
	LTC_ADCV(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL);//ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
	HAL_Delay(NORMAL_DELAY);	//FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;
	Read_Cell_Volt((uint16_t*) read_volt);
//	printf("volt end\n");
}

void Read_Thermistor(uint8_t tempindex, uint16_t *read_temp, uint16_t *read_auxreg) {
//	printf("Temperature read start\n");
	LTC_WRCOMM(NUM_DEVICES, BMS_MUX[tempindex]);
	LTC_STCOMM(2);
	//end sending to mux to read temperatures
	LTC_ADAX(MD_NORMAL, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
	HAL_Delay(NORMAL_DELAY); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;
	if (!Read_GPIO((uint16_t*) read_auxreg)) // Set to read back all aux registers
			{
		for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
			// Assuming data format is [cell voltage, cell voltage, ..., PEC, PEC]
			// PEC for each device is the last two bytes of its data segment
			uint16_t data = read_auxreg[dev_idx * NUM_AUX_GROUP];
			//read_temp[dev_idx * NUM_THERM_PER_MOD + tempindex] = data;
			Thermistor_To_Celsius(dev_idx, tempindex, (uint16_t*) read_temp, data); //+5 because vref is the last reg
	}
	}
//	printf("Temperature read end\n");
}

void Read_Pressure(uint16_t *read_pressure, uint16_t *read_auxreg) {
    LTC_WRCOMM(NUM_DEVICES, BMS_MUX[1]);
    LTC_STCOMM(2);

    LTC_ADAX(MD_NORMAL, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
    HAL_Delay(NORMAL_DELAY); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;

    if (!Read_GPIO((uint16_t*) read_auxreg)) {
    	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
        uint16_t data = read_auxreg[dev_idx * NUM_AUX_GROUP];
        ADC_To_Pressure(dev_idx, read_pressure, data);
    	}
    }
}

void Read_Atmos_Temp(uint16_t *v, uint16_t *read_auxreg) {
    LTC_WRCOMM(NUM_DEVICES, BMS_MUX[1]);
    LTC_STCOMM(2);

    LTC_ADAX(MD_NORMAL, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
    HAL_Delay(NORMAL_DELAY); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;

    if (!Read_GPIO((uint16_t*) read_auxreg)) {
    	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
        uint16_t data = read_auxreg[dev_idx * NUM_AUX_GROUP];
        Atmos_Temp_To_Celsius(dev_idx, read_pressure, data);
    	}
    }

    Get_AVG_Atmos_Temp();
}

void Read_Humidity(uint16_t *read_humidity, uint16_t *read_auxreg) {
    LTC_WRCOMM(NUM_DEVICES, BMS_MUX[1]);
    LTC_STCOMM(2);

    LTC_ADAX(MD_NORMAL, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
    HAL_Delay(NORMAL_DELAY); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;

    if (!Read_GPIO((uint16_t*) read_auxreg)) {
    	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
        uint16_t data = read_auxreg[dev_idx * NUM_AUX_GROUP];
        ADC_To_Humidity(dev_idx, read_pressure, data);
    	}
    }

    Get_AVG_Humidity();
}

void Get_Due_point(){

}

