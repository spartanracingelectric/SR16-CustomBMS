#include "print.h"
#include "usart.h"
//void print(uint8_t len, uint16_t *read_temp) {
//    char buf[128]; //buffer
//    uint16_t buf_length;
//
//    for (uint8_t i = 0; i < len; i++) {
//
//        buf_length = sprintf(buf, "C%u:%u/10000\n", i + 1, read_temp[i]);
//
//
//        HAL_UART_Transmit(&huart1, (uint8_t *)buf, buf_length, HAL_MAX_DELAY);
//    }
//
//
//    char newline[] = "\n";
//    HAL_UART_Transmit(&huart1, (uint8_t *)newline, sizeof(newline) - 1, HAL_MAX_DELAY);
//}
