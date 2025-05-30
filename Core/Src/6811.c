#include "6811.h"
#include "spi.h"
#include "main.h"

uint8_t wrpwm_buffer[4 + (8 * NUM_DEVICES)];
uint8_t wrcfg_buffer[4 + (8 * NUM_DEVICES)];
uint8_t wrcomm_buffer[4 + (8 * NUM_DEVICES)];

static const uint16_t LTC_CMD_RDCV[4] = { LTC_CMD_RDCVA, LTC_CMD_RDCVB,
LTC_CMD_RDCVC, LTC_CMD_RDCVD };

static const uint16_t LTC_CMD_AUXREG[2] = { LTC_CMD_RDAUXA, LTC_CMD_RDAUXB };

/* Wake LTC up from IDLE state into READY state */
void Wakeup_Idle(void) {
	uint8_t hex_ff = 0xFF;
	for (int i = 0; i < NUM_DEVICES; i++) {
		LTC_nCS_Low();							   // Pull CS low
		HAL_SPI_Transmit(&hspi1, &hex_ff, 1, 100); // Send byte 0xFF to wake LTC up
		LTC_nCS_High();							   // Pull CS high
	}
}

// wake up sleep
void Wakeup_Sleep(void) {

	for (int i = 0; i < NUM_DEVICES; i++) {
		LTC_nCS_Low();
		HAL_Delay(1);
		LTC_nCS_High();
		HAL_Delay(1);
	}
}

/* Read and store raw cell voltages at uint8_t 2d pointer */
LTC_SPI_StatusTypeDef LTC_getCellVoltages(uint16_t *read_voltages) {
	LTC_SPI_StatusTypeDef ret = LTC_SPI_OK;
	LTC_SPI_StatusTypeDef hal_ret;
	const uint8_t ARR_SIZE_REG = NUM_DEVICES * REG_LEN;
	uint8_t read_voltages_reg[ARR_SIZE_REG]; // Increased in size to handle multiple devices

	for (uint8_t i = 0; i < (NUM_CELL_SERIES_GROUP / LTC_SERIES_GROUPS_PER_RDCV);
			i++) {
		uint8_t cmd[4];
		uint16_t cmd_pec;

		cmd[0] = (0xFF & (LTC_CMD_RDCV[i] >> 8)); // RDCV Register
		cmd[1] = (0xFF & (LTC_CMD_RDCV[i]));	  // RDCV Register
		cmd_pec = LTC_Pec15_Calc(2, cmd);
		cmd[2] = (uint8_t) (cmd_pec >> 8);
		cmd[3] = (uint8_t) (cmd_pec);

		Wakeup_Idle(); // Wake LTC up

		LTC_nCS_Low(); // Pull CS low

		hal_ret = HAL_SPI_Transmit(&hspi1, (uint8_t*) cmd, 4, 100);
		if (hal_ret) {									// Non-zero means error
			ret |= (1 << (hal_ret + LTC_SPI_TX_BIT_OFFSET)); // TX error
		}

		hal_ret = HAL_SPI_Receive(&hspi1, (uint8_t*) read_voltages_reg,
				ARR_SIZE_REG, 100);
		if (hal_ret) {									// Non-zero means error
			ret |= (1 << (hal_ret + LTC_SPI_RX_BIT_OFFSET)); // RX error
		}
		LTC_nCS_High(); // Pull CS high

		// Process the received data
		for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
			// Assuming data format is [cell voltage, cell voltage, ..., PEC, PEC]
			// PEC for each device is the last two bytes of its data segment
			uint8_t *data_ptr = &read_voltages_reg[dev_idx * REG_LEN];
			// If PEC matches, copy the voltage data, omitting the PEC bytes
			memcpy(
					&read_voltages[dev_idx * NUM_CELL_SERIES_GROUP
							+ i * LTC_SERIES_GROUPS_PER_RDCV], data_ptr,
					REG_LEN - 2);
		}
	}

	return ret;
}

/**
 * 	write command to all pwm registers. This setup only allows to use 4b'1111 (HIGH) or 4b'0000 (LOW). 
 * @param total_ic		total count of ic (daisy chain)
 * @param pwm			A two dimensional array of the configuration data that will be written
 */
void LTC_writePWM(uint8_t total_ic, uint8_t pwm) {
	// NOTE currently chaging this method to only assign a specific PWM to all registers

	// TODO change it back to relying on @param pwm for duty cycle assignment. 

	const uint8_t BYTES_IN_REG = 6;
	const uint8_t CMD_LEN = 4 + (8 * total_ic);
	uint16_t pwm_pec;
	uint16_t cmd_pec;
	uint8_t cmd_index; // command counter

	// init bits
	wrpwm_buffer[0] = 0x00;
	wrpwm_buffer[1] = 0x20;
	cmd_pec = LTC_Pec15_Calc(2, wrpwm_buffer);
	wrpwm_buffer[2] = (uint8_t) (cmd_pec >> 8);
	wrpwm_buffer[3] = (uint8_t) (cmd_pec);

	cmd_index = 4;				// Command bits
	for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--) // executes for each ltc6811 in daisy chain, this loops starts with
			{
		// the last IC on the stack. The first configuration written is
		// received by the last IC in the daisy chain

		for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG;
				current_byte++) // executes for each of the 6 bytes in the CFGR register
				{
			// current_byte is the byte counter

			wrpwm_buffer[cmd_index] = pwm; //adding the pwm data to the array to be sent
			cmd_index = cmd_index + 1;
		}

		pwm_pec = (uint16_t) LTC_Pec15_Calc(BYTES_IN_REG, &pwm); // calculating the PEC for each ICs configuration register data
		wrpwm_buffer[cmd_index] = (uint8_t) (pwm_pec >> 8);
		wrpwm_buffer[cmd_index + 1] = (uint8_t) pwm_pec;
		cmd_index = cmd_index + 2;
	}

	Wakeup_Idle(); // This will guarantee that the ltc6811 isoSPI port is awake.This command can be removed.
	LTC_nCS_Low();
	HAL_SPI_Transmit(&hspi1, (uint8_t*) wrpwm_buffer, CMD_LEN, 100);
	LTC_nCS_High();
}

void LTC_writeCFG(uint8_t total_ic, //The number of ICs being written to
		uint8_t config[][6] //A two dimensional array of the configuration data that will be written
		) {
	const uint8_t BYTES_IN_REG = 6;
	const uint8_t CMD_LEN = 4 + (8 * total_ic);
	uint16_t cfg_pec;
	uint8_t cmd_index; //command counter

	wrcfg_buffer[0] = 0x00;
	wrcfg_buffer[1] = 0x01;
	wrcfg_buffer[2] = 0x3d;
	wrcfg_buffer[3] = 0x6e;

	cmd_index = 4;
	// executes for each ltc6811 in daisy chain, this loops starts with
	for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--) {
		// the last IC on the stack. The first configuration written is
		// received by the last IC in the daisy chain

		// executes for each of the 6 bytes in the CFGR register
		for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG;
				current_byte++) {
			// current_byte is the byte counter

			wrcfg_buffer[cmd_index] = config[current_ic - 1][current_byte]; //adding the config data to the array to be sent
			cmd_index = cmd_index + 1;
		}

		cfg_pec = (uint16_t) LTC_Pec15_Calc(BYTES_IN_REG,
				&config[current_ic - 1][0]); // calculating the PEC for each ICs configuration register data
		wrcfg_buffer[cmd_index] = (uint8_t) (cfg_pec >> 8);
		wrcfg_buffer[cmd_index + 1] = (uint8_t) cfg_pec;
		cmd_index = cmd_index + 2;
	}

	Wakeup_Idle(); // This will guarantee that the ltc6811 isoSPI port is awake.This command can be removed.
	LTC_nCS_Low();
	HAL_SPI_Transmit(&hspi1, (uint8_t*) wrcfg_buffer, CMD_LEN, 100);
	LTC_nCS_High();
}

/**
 * 
 * @param total_ic	The number of ICs being written to
 * @param comm[6]	A two dimensional array of the comm data that will be written
 */
void LTC_SPI_writeCommunicationSetting(uint8_t total_ic, uint8_t comm[6]) {
	const uint8_t BYTES_IN_REG = 6;
	const uint8_t CMD_LEN = 4 + (8 * total_ic);
	uint16_t comm_pec;
	uint16_t cmd_pec;
	uint8_t cmd_index; // command counter

	wrcomm_buffer[0] = 0x07;
	wrcomm_buffer[1] = 0x21;
	cmd_pec = LTC_Pec15_Calc(2, wrcomm_buffer);
	wrcomm_buffer[2] = (uint8_t) (cmd_pec >> 8);
	wrcomm_buffer[3] = (uint8_t) (cmd_pec);

	cmd_index = 4;
	for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--) // executes for each ltc6811 in daisy chain, this loops starts with
			{
		// the last IC on the stack. The first configuration written is
		// received by the last IC in the daisy chain

		for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG;
				current_byte++) // executes for each of the 6 bytes in the CFGR register
				{
			// current_byte is the byte counter
			wrcomm_buffer[cmd_index] = comm[current_byte]; // adding the config data to the array to be sent
			cmd_index = cmd_index + 1;
		}
		comm_pec = (uint16_t) LTC_Pec15_Calc(BYTES_IN_REG, &comm[0]); // calculating the PEC for each ICs configuration register data
		wrcomm_buffer[cmd_index] = (uint8_t) (comm_pec >> 8);
		wrcomm_buffer[cmd_index + 1] = (uint8_t) comm_pec;
		cmd_index = cmd_index + 2;
	}

	Wakeup_Idle(); // This will guarantee that the ltc6811 isoSPI port is awake.This command can be removed.
	LTC_nCS_Low();
	HAL_SPI_Transmit(&hspi1, (uint8_t*) wrcomm_buffer, CMD_LEN, 100);
	LTC_nCS_High();
}

/**
 * Shifts data in COMM register out over ltc6811 SPI/I2C port
 */
void LTC_SPI_requestData(uint8_t len) {

	uint8_t cmd[4];
	uint16_t cmd_pec;

	cmd[0] = 0x07;
	cmd[1] = 0x23;
	cmd_pec = LTC_Pec15_Calc(2, cmd);
	cmd[2] = (uint8_t) (cmd_pec >> 8);
	cmd[3] = (uint8_t) (cmd_pec);

	Wakeup_Idle(); // This will guarantee that the ltc6811 isoSPI port is awake. This command can be removed.
	LTC_nCS_Low();
	HAL_SPI_Transmit(&hspi1, (uint8_t*) cmd, 4, 100);
	for (int i = 0; i < len * 3; i++) {
		HAL_SPI_Transmit(&hspi1, (uint8_t*) 0xFF, 1, 100);
	}
	LTC_nCS_High();
}

LTC_SPI_StatusTypeDef LTC_readGPIOs(uint16_t *read_auxiliary) {
	LTC_SPI_StatusTypeDef ret = LTC_SPI_OK;
	LTC_SPI_StatusTypeDef hal_ret;
	const uint8_t ARR_SIZE_REG = NUM_DEVICES * REG_LEN;
	uint8_t read_auxiliary_reg[ARR_SIZE_REG]; // Increased in size to handle multiple devices

	for (uint8_t i = 0;
			i < (NUM_AUX_SERIES_GROUPS / LTC_SERIES_GROUPS_PER_RDAUX); i++) {
		uint8_t cmd[4];
		uint16_t cmd_pec;

		cmd[0] = (0xFF & (LTC_CMD_AUXREG[i] >> 8)); // RDCV Register
		cmd[1] = (0xFF & (LTC_CMD_AUXREG[i]));		// RDCV Register
		cmd_pec = LTC_Pec15_Calc(2, cmd);
		cmd[2] = (uint8_t) (cmd_pec >> 8);
		cmd[3] = (uint8_t) (cmd_pec);

		Wakeup_Idle(); // Wake LTC up

		LTC_nCS_Low(); // Pull CS low

		hal_ret = HAL_SPI_Transmit(&hspi1, (uint8_t*) cmd, 4, 100);
		if (hal_ret) {									// Non-zero means error
			ret |= (1 << (hal_ret + LTC_SPI_TX_BIT_OFFSET)); // TX error
		}

		hal_ret = HAL_SPI_Receive(&hspi1, (uint8_t*) read_auxiliary_reg,
				ARR_SIZE_REG, 100);
		if (hal_ret) {									// Non-zero means error
			ret |= (1 << (hal_ret + LTC_SPI_RX_BIT_OFFSET)); // RX error
		}

		LTC_nCS_High(); // Pull CS high

		// Process the received data
		for (uint8_t dev_idx = 0; dev_idx < NUM_DEVICES; dev_idx++) {
			// Assuming data format is [cell voltage, cell voltage, ..., PEC, PEC]
			// PEC for each device is the last two bytes of its data segment
			uint8_t *data_ptr = &read_auxiliary_reg[dev_idx * REG_LEN];

			memcpy(
					&read_auxiliary[dev_idx * NUM_AUX_SERIES_GROUPS
							+ i * LTC_SERIES_GROUPS_PER_RDAUX], data_ptr,
					REG_LEN - 2);
		}

	}

	return ret;
}

/*
 Starts cell voltage conversion
 */
void LTC_startADCVoltage(uint8_t MD,  // ADC Mode
		uint8_t DCP, // Discharge Permit
		uint8_t CH   // Cell Channels to be measured
		) {
	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x02;
	md_bits = (MD & 0x01) << 7;
	cmd[1] = md_bits + 0x60 + (DCP << 4) + CH;
	cmd_pec = LTC_Pec15_Calc(2, cmd);
	cmd[2] = (uint8_t) (cmd_pec >> 8);
	cmd[3] = (uint8_t) (cmd_pec);

	Wakeup_Idle(); // This will guarantee that the ltc6811 isoSPI port is awake. This command can be removed.
	LTC_nCS_Low();
	HAL_SPI_Transmit(&hspi1, (uint8_t*) cmd, 4, 100);
	LTC_nCS_High();
}

void LTC_startADC_GPIO(uint8_t MD, // ADC Mode
		uint8_t CHG // GPIO Channels to be measured)
		) {
	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] = md_bits + 0x60 + CHG;
	cmd_pec = LTC_Pec15_Calc(2, cmd);
	cmd[2] = (uint8_t) (cmd_pec >> 8);
	cmd[3] = (uint8_t) (cmd_pec);

	/*
	 Wakeup_Idle (); //This will guarantee that the ltc6811 isoSPI port is awake. This command can be removed.
	 output_low(LTC6811_CS);
	 spi_write_array(4,cmd);
	 output_high(LTC6811_CS);
	 */
	Wakeup_Idle(); // This will guarantee that the ltc6811 isoSPI port is awake. This command can be removed.
	LTC_nCS_Low();
	HAL_SPI_Transmit(&hspi1, (uint8_t*) cmd, 4, 100);
	LTC_nCS_High();
}

int32_t LTC_POLLADC() {
	uint32_t start_time = HAL_GetTick();
	uint8_t finished = 0;
	uint8_t adc_status = 0xFF;	//initialize adc status
	uint8_t dummy_tx = 0x00;	//we need to send this to give clock LTC6811 so it can send data
	uint8_t cmd[4];
	uint16_t cmd_pec;
	//get ready for the PLADC command
	cmd[0] = 0x07;
	cmd[1] = 0x14;
	cmd_pec = LTC_Pec15_Calc(2, cmd);
	cmd[2] = (uint8_t) (cmd_pec >> 8);
	cmd[3] = (uint8_t) (cmd_pec);

	Wakeup_Idle(); // This will guarantee that the ltc6811 isoSPI port is awake. This command can be removed.
	//Send PLADC command to LTC6811
	LTC_nCS_Low();
	HAL_SPI_Transmit(&hspi1, (uint8_t*) cmd, 4, 100);
	LTC_nCS_High();
	//Check if ADC is done
	while (((HAL_GetTick() - start_time) < 210000)) {  // timeout at 210ms
		LTC_nCS_Low();
		HAL_SPI_Transmit(&hspi1, &dummy_tx, 1, 100);  //Send dummy byte and get adc status
		HAL_SPI_Receive(&hspi1, &adc_status, 1, 100);
		LTC_nCS_High();
		if (adc_status != 0xFF) {  // if it's not 0xFF, finish adc
			finished = 1;
			break;
		} else {
			HAL_Delay(1);  //delay 1ms
		}
	    }

	LTC_nCS_High();
	return (finished ? (HAL_GetTick() - start_time) : 0);
}



/* Read and store raw cell voltages at uint8_t 2d pointer */
int Calc_Pack_Voltage(uint16_t *read_voltages) {
	int packvoltage = 0;
	for (int i = 0; i < NUM_DEVICES * NUM_CELL_SERIES_GROUP; i++) {
		packvoltage += read_voltages[i];
	}
	return packvoltage;
}

static const unsigned int crc15Table[256] = { 0x0, 0xc599, 0xceab, 0xb32,
		0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e,
		0x3aac, //!< precomputed CRC15 Table
		0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5,
		0x7558, 0xb0c1, 0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f,
		0x44c6, 0x4ff4, 0x8a6d, 0x5b2e, 0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678,
		0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d,
		0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544,
		0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5,
		0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b,
		0xc969, 0xcf0, 0xdf0d, 0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9,
		0x5560, 0x869d, 0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167,
		0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024,
		0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
		0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318,
		0xd681, 0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286,
		0xa213, 0x678a, 0x6cb8, 0xa921, 0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614,
		0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9, 0xf84, 0xca1d,
		0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
		0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f,
		0x21f2, 0xe46b, 0xef59, 0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5,
		0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc,
		0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
		0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b,
		0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41,
		0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x846, 0xcddf,
		0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453,
		0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd,
		0x2630, 0xe3a9, 0xe89b, 0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0,
		0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 0x585a, 0x8ba7, 0x4e3e,
		0x450c, 0x8095 };

/**
 * error calculation and handling for poor command use. 
 * @param 	len		Number of bytes that will be used to calculate a PEC
 * @param	data	Array of data that will be used to calculate a PEC
 */
uint16_t LTC_Pec15_Calc(uint8_t len, uint8_t *data) {
	uint16_t remainder, addr;
	remainder = 16; // Initialize the PEC to 0x10000

	for (uint8_t i = 0; i < len; i++) // loops for each byte in data array
			{
		addr = ((remainder >> 7) ^ data[i]) & 0xff; // calculate PEC table address
		remainder = (remainder << 8) ^ crc15Table[addr];
	}

	return (remainder * 2); // The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}
