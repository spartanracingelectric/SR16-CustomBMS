#include "module.h"
#include <math.h>
#include "6811.h"
#include <stdio.h>
#include "usart.h"

#define ntcNominal 10000.0f
#define ntcSeriesResistance 10000.0f
#define ntcBetaFactor 3435.0f
#define ntcNominalTemp 25.0f

static const float invNominalTemp = 1.0f / (ntcNominalTemp + 273.15f);
static const float invBetaFactor = 1.0f / ntcBetaFactor;

// addresses provided by the mux (uses i2c protocol)
static uint8_t BMS_MUX[][6] = {{ 0x69, 0x28, 0x0F, 0xF9, 0x7F, 0xF9 }, { 0x69, 0x28, 0x0F, 0xE9, 0x7F, 0xF9 },
								 { 0x69, 0x28, 0x0F, 0xD9, 0x7F, 0xF9 }, { 0x69, 0x28, 0x0F, 0xC9, 0x7F, 0xF9 },
								 { 0x69, 0x28, 0x0F, 0xB9, 0x7F, 0xF9 }, { 0x69, 0x28, 0x0F, 0xA9, 0x7F, 0xF9 },
								 { 0x69, 0x28, 0x0F, 0x99, 0x7F, 0xF9 }, { 0x69, 0x28, 0x0F, 0x89, 0x7F, 0xF9 },
								 { 0x69, 0x08, 0x0F, 0xF9, 0x7F, 0xF9 }, { 0x69, 0x08, 0x0F, 0xE9, 0x7F, 0xF9 },
								 { 0x69, 0x08, 0x0F, 0xD9, 0x7F, 0xF9 }, { 0x69, 0x08, 0x0F, 0xC9, 0x7F, 0xF9 },
							 	 { 0x69, 0x08, 0x0F, 0xB9, 0x7F, 0xF9 }, { 0x69, 0x08, 0x0F, 0xA9, 0x7F, 0xF9 },
								 { 0x69, 0x08, 0x0F, 0x99, 0x7F, 0xF9 }, { 0x69, 0x08, 0x0F, 0x89, 0x7F, 0xF9 } };

void ADC_To_Pressure(uint8_t dev_idx, uint16_t *pressure, uint16_t adc_data) {
    float psi = (float) adc_data / 325.0;  // convert the adc value based on Vref

    pressure[dev_idx] = (uint16_t)(psi * 100);  // relative to atmospheric pressure
}

void Atmos_Temp_To_Celsius(uint8_t dev_idx, uint16_t *read_atmos_temp, uint16_t adc_data) {
    float voltage_ratio = (float) adc_data / LTC6811_Vdd;  // convert the adc value based on Vref

    float temperature_value = -66.875 + 218.75 * voltage_ratio;  //Calculate pressure

    read_atmos_temp[dev_idx] = (uint16_t)temperature_value;  // 圧力値を整数に変換
}

void ADC_To_Humidity(uint8_t dev_idx, uint16_t *humidity, uint16_t adcValue) {
    float voltage_ratio = (float)adcValue / LTC6811_Vdd;

    float humidity_value = (-12.5 + 125.0 * voltage_ratio);  //Calculate pressure

    humidity[dev_idx] = (uint16_t)(humidity_value);  // 圧力値を整数に変換
}

void Get_Actual_Temps(uint8_t dev_idx, uint8_t tempindex, uint16_t *actual_temp, uint16_t data) {
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

void Read_Volt(uint16_t *read_volt) {
//	printf("volt start\n");
	LTC_startADCVoltage(MD_NORMAL, DCP_DISABLED, CELL_CH_ALL);//ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
	HAL_Delay(NORMAL_DELAY);	//FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;
	LTC_getCellVoltages((uint16_t*) read_volt);
//	printf("volt end\n");
}

void Read_Temp(uint8_t tempindex, uint16_t *read_temp, uint16_t *read_auxreg) {

//	printf("Temperature read start\n");
	LTC_SPI_writeCommunicationSetting(NUM_DEVICES, BMS_MUX[tempindex]);
	LTC_SPI_requestData(2);
	//end sending to mux to read temperatures
	if (tempindex == 0) {
		LTC_startADC_GPIO(MD_NORMAL, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
		HAL_Delay(NORMAL_DELAY + 2); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;
	} else {
		LTC_startADC_GPIO(MD_FAST, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
		HAL_Delay(FAST_DELAY); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;
	}
	if (!LTC_readGPIOs((uint16_t*) read_auxreg)) // Set to read back all aux registers
			{
		for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
			//Wakeup_Idle();
			// Assuming data format is [cell voltage, cell voltage, ..., PEC, PEC]
			// PEC for each device is the last two bytes of its data segment
			uint16_t data = read_auxreg[dev_idx * NUM_AUX_GROUP];
			//read_temp[dev_idx * NUM_THERM_PER_MOD + tempindex] = data;
			Get_Actual_Temps(dev_idx, tempindex, (uint16_t*) read_temp, data); //+5 because vref is the last reg
		}
	}
//	printf("Temperature read end\n");
}


void Read_Pressure(batteryModule *batt) {
	LTC_SPI_writeCommunicationSetting(NUM_DEVICES, BMS_MUX[12]);
	LTC_SPI_requestData(2);

	LTC_startADC_GPIO(MD_NORMAL, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
    HAL_Delay(NORMAL_DELAY); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;

    if (!LTC_readGPIOs((uint16_t*) batt->read_auxreg)) {
    	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
            uint16_t data = batt->read_auxreg[dev_idx * NUM_AUX_GROUP];
            ADC_To_Pressure(dev_idx, batt->pressure, data);
    	}
    }
}

void Read_Atmos_Temp(batteryModule *batt) {
	LTC_SPI_writeCommunicationSetting(NUM_DEVICES, BMS_MUX[14]);
	LTC_SPI_requestData(2);

	LTC_startADC_GPIO(MD_NORMAL, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
    HAL_Delay(NORMAL_DELAY); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;

    if (!LTC_readGPIOs((uint16_t*) batt->read_auxreg)) {

    	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {

            uint16_t data = batt->read_auxreg[dev_idx * NUM_AUX_GROUP];
            Atmos_Temp_To_Celsius(dev_idx, batt->atmos_temp, data);

    	}
    }
}


// this function calculates dew point from the properties of a battery module object
// Dew Point: temperature (in Celsius) that air needs to be cooled to reach 100% humidity
void Get_Dew_Point(batteryModule *batt) {

	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {

		uint16_t humidity = batt->humidity[dev_idx];
		uint16_t atmos_temp = batt->atmos_temp[dev_idx];

		// simple approximation that is accurate to within 1 deg C
		// Only works well when Relative Humidity is above 50%

		batt->dew_point[dev_idx] = atmos_temp - ((float)(100 - humidity) / 5);

	}

}

void Read_Humidity(batteryModule *batt) {
	LTC_SPI_writeCommunicationSetting(NUM_DEVICES, BMS_MUX[13]);
	LTC_SPI_requestData(2);

	LTC_startADC_GPIO(MD_NORMAL, 1); //ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
    HAL_Delay(NORMAL_DELAY); //FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;

    if (!LTC_readGPIOs((uint16_t*) batt->read_auxreg)) {
    	for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
        uint16_t data = batt->read_auxreg[dev_idx * NUM_AUX_GROUP];
        ADC_To_Humidity(dev_idx, batt->humidity, data);
    	}
    }
}




