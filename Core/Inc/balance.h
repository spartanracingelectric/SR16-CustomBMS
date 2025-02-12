#ifndef INC_BALANCE_H_
#define INC_BALANCE_H_

#include <stdint.h>

void Balance_start(uint16_t *read_volt, uint8_t length, uint16_t lowest);
void Balance_end(uint8_t *faults);

#endif /* INC_BALANCE_H_ */
