/**
 ******************************************************************************
 * File Name          : USART.c
 * Description        : This file provides code for the configuration
 *                      of the USART instances.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <h7_modules.h>
#include <stm32h747xx.h>

UART_HandleTypeDef huart;

/* UART4 init function */
void MX_UART4_Init(void) {
    huart.Instance                    = UART4;
    huart.Init.BaudRate               = 115200;
    huart.Init.WordLength             = UART_WORDLENGTH_8B;
    huart.Init.StopBits               = UART_STOPBITS_1;
    huart.Init.Parity                 = UART_PARITY_NONE;
    huart.Init.Mode                   = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if(HAL_UART_Init(&huart) != HAL_OK) { Error_Handler(); }
    if(HAL_UARTEx_SetTxFifoThreshold(&huart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) { Error_Handler(); }
    if(HAL_UARTEx_SetRxFifoThreshold(&huart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) { Error_Handler(); }
    if(HAL_UARTEx_DisableFifoMode(&huart) != HAL_OK) { Error_Handler(); }
}

/* USER CODE BEGIN 0 */

void MX_UART_Init(void) { MX_UART4_Init(); }

/* USER CODE END 0 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
