#ifndef __h7_modules_H
#define __h7_modules_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32h7xx.h>

void Error_Handler(void);

extern UART_HandleTypeDef huart;
void MX_UART_Init(void);

void MX_GPIO_Init(void);


#ifdef __cplusplus
}
#endif

#endif
