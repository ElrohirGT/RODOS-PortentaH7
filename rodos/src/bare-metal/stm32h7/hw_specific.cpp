/**
 * @file hw_specific.cc
 * @date 2008/04/23 7:33
 * @port 2010/02/18 19:14
 * @author Miroslaw Sulejczak, modified by Michael Ruffer
 *
 *
 * @brief all hardware specific stuff I have no better place for ...
 *
 */

#include <sys/stat.h>

#include "rodos.h"

#include "default-platform-parameter.h"
#include "hw_specific.h"
#include "hal/hal_uart.h"

// #include "stm32h747xx.h"
#include "stm32h7xx_hal.h"

volatile long *contextT;

namespace RODOS {

void TIMx_init(); // timer for system time -> see hw_timer.cpp

void hwInit() {
	__HAL_RCC_HSEM_CLK_ENABLE();

  // Update the SystemCoreClock variable.
  SystemCoreClockUpdate();
  HAL_Init();

	TIMx_init(); // Timer for system time
}

#ifndef NO_RODOS_NAMESPACE
}
#endif

