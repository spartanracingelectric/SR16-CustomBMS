#include "accumulator.h"

#include <math.h>
#include <stdio.h>

#include "6811.h"
#include "stm32f1xx_hal.h"

#define ntcNominal 10000.0f
#define ntcSeriesResistance 10000.0f
#define ntcBetaFactor 3435.0f
#define ntcNominalTemp 25.0f

void Accumulator_performSteinHartConversion(uint8_t dev_idx, uint8_t tempindex,
                                            uint16_t *actual_temp,
                                            uint16_t data);

const float invNominalTemp = 1.0f / (ntcNominalTemp + 273.15f);
const float invBetaFactor = 1.0f / ntcBetaFactor;

static uint8_t BMS_THERM[][6] = {
    {0x69, 0x28, 0x0F, 0xF9, 0x7F, 0xF9}, {0x69, 0x28, 0x0F, 0xE9, 0x7F, 0xF9},
    {0x69, 0x28, 0x0F, 0xD9, 0x7F, 0xF9}, {0x69, 0x28, 0x0F, 0xC9, 0x7F, 0xF9},
    {0x69, 0x28, 0x0F, 0xB9, 0x7F, 0xF9}, {0x69, 0x28, 0x0F, 0xA9, 0x7F, 0xF9},
    {0x69, 0x28, 0x0F, 0x99, 0x7F, 0xF9}, {0x69, 0x28, 0x0F, 0x89, 0x7F, 0xF9},
    {0x69, 0x08, 0x0F, 0xF9, 0x7F, 0xF9}, {0x69, 0x08, 0x0F, 0xE9, 0x7F, 0xF9},
    {0x69, 0x08, 0x0F, 0xD9, 0x7F, 0xF9}, {0x69, 0x08, 0x0F, 0xC9, 0x7F, 0xF9},
    {0x69, 0x08, 0x0F, 0xB9, 0x7F, 0xF9}, {0x69, 0x08, 0x0F, 0xA9, 0x7F, 0xF9},
    {0x69, 0x08, 0x0F, 0x99, 0x7F, 0xF9}, {0x69, 0x08, 0x0F, 0x89, 0x7F, 0xF9}};

void Accumulator_readVoltages(uint16_t *dest) {
    LTC_ADCV(MD_NORMAL, DCP_DISABLED,
             CELL_CH_ALL);    // ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
    HAL_Delay(NORMAL_DELAY);  // FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;
    Read_Cell_Volt((uint16_t *)dest);
}

void Accumulator_readTempuratures(uint8_t tempindex, uint16_t *read_temp,
                                  uint16_t *read_auxreg) {
    LTC_WRCOMM(NUM_DEVICES, BMS_THERM[tempindex]);
    LTC_STCOMM(2);
    // end sending to mux to read temperatures
    LTC_ADAX(MD_NORMAL, 1);   // ADC mode: MD_FILTERED, MD_NORMAL, MD_FAST
    HAL_Delay(NORMAL_DELAY);  // FAST_DELAY, NORMAL_DELAY, FILTERD_DELAY;
    if (!Read_Cell_Temps(
            (uint16_t *)read_auxreg))  // Set to read back all aux registers
    {
        for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
            // Wakeup_Idle();
            //  Assuming data format is [cell voltage, cell voltage, ..., PEC,
            //  PEC] PEC for each device is the last two bytes of its data
            //  segment
            uint16_t data = read_auxreg[dev_idx * NUM_AUX_GROUP];
            // read_temp[dev_idx * NUM_THERM_PER_MOD + tempindex] = data;
            Accumulator_performSteinHartConversion(
                dev_idx, tempindex, (uint16_t *)read_temp,
                data);  //+5 because vref is the last reg
        }
    }
}

void Accumulator_performSteinHartConversion(uint8_t dev_idx, uint8_t tempindex,
                                            uint16_t *actual_temp,
                                            uint16_t data) {
    if (data == 0) {
        actual_temp[dev_idx * NUM_THERM_PER_MOD + tempindex] =
            999.0f;  // error value
        return;
    }

    float scalar = 30000.0f / (float)(data)-1.0f;
    scalar = ntcSeriesResistance / scalar;

    float steinhart = scalar / ntcNominal;
    steinhart = log(steinhart);
    steinhart *= invBetaFactor;
    steinhart += invNominalTemp;
    steinhart = 1.0f / steinhart;
    steinhart -= 273.15f;

    actual_temp[dev_idx * NUM_THERM_PER_MOD + tempindex] = steinhart;
}
