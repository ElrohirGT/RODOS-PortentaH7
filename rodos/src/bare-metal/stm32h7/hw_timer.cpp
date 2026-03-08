/*
 * @file hw_timer.cpp
 * @date 2012/08/22
 * @author Michael Ruffer
 *
 * Copyright 2012 University of Wuerzburg
 *
 * @brief Timer for system time and preemption
 */
#include "rodos.h"
#include "rodos-atomic.h"
#include "hw_specific.h"

#include "default-platform-parameter.h"

#include "stm32h747xx.h"
#include "stm32h7xx_hal_tim.h"
#include "Legacy/stm32_hal_legacy.h"

#ifndef NO_RODOS_NAMESPACE
namespace RODOS {
#endif


/***** Preemption Timer - Cortex "SysTick" counter *****
 *
 * Timer to force scheduler calls -> preemption
 * - cortex system counter is used -> "SysTick"
 * - 24Bit counter running at CPU_CLK/8 (168MHz/8 = 21MHz)
 * -> max. value: 0xFFF = 2^24 = 16.777.216
 * -> 1ms @ 21MHz: 21.000
 * -> 10ms  @ 21MHz: 210.000
 * -> 100ms  @ 21MHz: 2.100.000
 * -> !!! 1s  @ 21MHz: 21.000.000 -> counter overflow !!!
 */
extern RODOS::Atomic<bool> yieldSchedulingLock;


extern "C" {
/*
 * Interrupt Service Routine for "SysTick" counter
 *
 * !!! Don't use "naked" for the ISR, because the system crashes
 * -> maybe because not all registers are saved automatically
 * -> it works when the compiler puts prologue and epilogue in the ISR
 *    -> but this can be a problem when ISRs can interrupt each other
 * -> this can happen when they don't have the same priority !!!
 */
void SysTick_Handler();
void SysTick_Handler() {
    if (yieldSchedulingLock == false) {
        /* request PendSV-interrupt (that calls the scheduler) */
        SCB->ICSR = SCB->ICSR | SCB_ICSR_PENDSVSET_Msk;
    }
}


/** \brief  System Tick Configuration

    This function initialises the system tick timer and its interrupt WITHOUT starting it
    Counter is in free running mode to generate periodical interrupts.

    \param [in]  ticks  Number of ticks between two interrupts
    \return          0  Function succeeded
    \return          1  Function failed
 */
static __INLINE uint32_t SysTick_Config_New(uint32_t ticks)
{
  if (ticks > SysTick_LOAD_RELOAD_Msk)  return (1);            /* Reload value impossible */

  SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;      /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Cortex-M0 System Interrupts */
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
//  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
//                   SysTick_CTRL_TICKINT_Msk   |
//                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                  /* Function successful */
}

static __INLINE uint32_t SysTick_IRQEnable(void)
{
	SysTick->CTRL = SysTick->CTRL | SysTick_CTRL_TICKINT_Msk;

	return (0);
}


static __INLINE uint32_t SysTick_IRQDisable(void)
{
	SysTick->CTRL = SysTick->CTRL & ~( SysTick_CTRL_TICKINT_Msk);

	return (0);
}

static __INLINE uint32_t SysTick_Enable(void)
{
	SysTick->CTRL = 	// SysTick_CTRL_CLKSOURCE_Msk	| // if set clock source = CPU_CLK else CPU_CLK/8
						SysTick_CTRL_TICKINT_Msk	|
						SysTick_CTRL_ENABLE_Msk;

	return (0);
}


static __INLINE uint32_t SysTick_Disable(void)
{
	SysTick->CTRL = 0;

	return (0);
}


} // end extern "C"

/**
* the timer interval
*/
long long Timer::microsecondsInterval = PARAM_TIMER_INTERVAL;

void Timer::init()
{
	SysTick_Config_New(static_cast<uint32_t>((SystemCoreClock/8) * Timer::microsecondsInterval / 1000000)); // initialization of systick timer
}

/**
* start timer
*/
void Timer::start()
{
	SysTick_Config_New(static_cast<uint32_t>((SystemCoreClock/8) * Timer::microsecondsInterval / 1000000)); // initialization of systick timer
	SysTick_Enable();
}

/**
* stop timer
*/
void Timer::stop()
{
	SysTick_Disable();
}

/**
* set timer interval
*/
void Timer::setInterval(const int64_t microsecondsInterval) {
  Timer::microsecondsInterval = microsecondsInterval;
}




/***** System Time ******
 *
 * timer is used to generate the system time
 * -> interrupt every 10ms
 * -> time resolution: 1/timerClock = 166,6ns
 * -> max. timerCount = timerClock/100 = 60 000 (IRQ every 10ms)
 */
static const unsigned int timerClock = 6000000;

/* timer 2,3,4,5,6,7,12,13,14 running at APB1_CLK*2 = 84MHz
 * timer 1,8,10,11 running at APB2_CLK*2 = 168MHz
 * timer 2,5: 32 Bit
 * timer 1,3,4,6,7,8,9,10,11,12,13,14: 16 Bit
 */
/* Timer 2: 32Bit, APB1 */
//#define TIMx                    TIM2
//#define RCC_APBnPeriph_TIMx     RCC_APB1Periph_TIM2
//#define DBGMCU_TIMx_STOP        DBGMCU_TIM2_STOP
//#define TIMx_IRQn               TIM2_IRQn
//#define TIMx_IRQHandler         TIM2_IRQHandler
//#define RCC_APBnPeriphClockCmd  RCC_APB1PeriphClockCmd
//#define RCC_APBnPeriphResetCmd  RCC_APB1PeriphResetCmd
//#define PCLKn_Frequency         PCLK1_Frequency

/* Timer 7: 16Bit, APB1 */
//#define TIMx                    TIM7
//#define RCC_APBnPeriph_TIMx     RCC_APB1Periph_TIM7
//#define DBGMCU_TIMx_STOP        DBGMCU_TIM7_STOP
//#define TIMx_IRQn               TIM7_IRQn
//#define TIMx_IRQHandler         TIM7_IRQHandler
//#define RCC_APBnPeriphClockCmd  RCC_APB1PeriphClockCmd
//#define RCC_APBnPeriphResetCmd  RCC_APB1PeriphResetCmd
//#define PCLKn_Frequency         PCLK1_Frequency

/* Timer 11: 16Bit, APB2 */

#define TIMx                    TIM12
// #define RCC_APBnPeriph_TIMx     RCC_APB2Periph_TIM12
// #define DBGMCU_TIMx_STOP        DBGMCU_TIM12_STOP
// #define TIMx_IRQn               TIM1_TRG_COM_TIM12_IRQn
// #define TIMx_IRQHandler         TIM1_TRG_COM_TIM12_IRQHandler
// #define RCC_APBnPeriphClockCmd  RCC_APB2PeriphClockCmd
// #define RCC_APBnPeriphResetCmd  RCC_APB2PeriphResetCmd
// #define PCLKn_Frequency         PCLK2_Frequency

/* Timer 14: 16Bit, APB1 */
//#define TIMx                    TIM14
//#define RCC_APBnPeriph_TIMx     RCC_APB1Periph_TIM14
//#define DBGMCU_TIMx_STOP        DBGMCU_TIM14_STOP
//#define TIMx_IRQn               TIM8_TRG_COM_TIM14_IRQn
//#define TIMx_IRQHandler         TIM8_TRG_COM_TIM14_IRQHandler
//#define RCC_APBnPeriphClockCmd  RCC_APB1PeriphClockCmd
//#define RCC_APBnPeriphResetCmd  RCC_APB1PeriphResetCmd
//#define PCLKn_Frequency         PCLK1_Frequency


int64_t nanoTime;

extern "C" {
/*
 * Interrupt Service Routine for Timer
 *
 * !!! Don't use "naked" for the ISR, because the system crashes
 * -> maybe because not all registers are saved automatically
 * -> it works when the compiler puts prologue and epilogue in the ISR
 *    -> but this can be a problem when ISRs can interrupt each other
 * -> this can happen when they don't have the same priority !!!
 */
// void TIMx_IRQHandler();
// void TIMx_IRQHandler()
// {
//    TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
//    NVIC_ClearPendingIRQ(TIMx_IRQn);
//    nanoTime += 10000000; // 10M ns for each 10ms-tick
// }
// } // end extern "C"

/* Capture Compare buffer */
/* 32-bytes Alignment is needed for cache maintenance purpose */
ALIGN_32BYTES( uint32_t aCCValue_Buffer[8])= {0};

/* Timer Period*/
uint32_t uwTimerPeriod  = 0;

void TIMx_init();
void TIMx_init(){
    TIM_HandleTypeDef    TimHandle;
		TIM_OC_InitTypeDef sConfig;

/* Initialize TIM3 peripheral as follows:
      + Period = TimerPeriod (To have an output frequency equal to 17.570 KHz)
      + Repetition Counter = 3
      + Prescaler = 0
      + ClockDivision = 0
      + Counter direction = Up
  */
  TimHandle.Instance = TIMx;

  TimHandle.Init.Period            = uwTimerPeriod;
  TimHandle.Init.RepetitionCounter = 3;
  TimHandle.Init.Prescaler         = 0;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-2- Configure the PWM channel 1 ########################################*/
  sConfig.OCMode       = TIM_OCMODE_PWM1;
  sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfig.Pulse        = aCCValue_Buffer[0];
  sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
  sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Configuration Error */
    Error_Handler();
  }

  /*##-3- Start PWM signal generation in DMA mode ############################*/
  if (HAL_TIM_PWM_Start_DMA(&TimHandle, TIM_CHANNEL_1, aCCValue_Buffer, 3) != HAL_OK)
  {
    /* Starting Error */
    Error_Handler();
  }
}
}

int64_t hwGetNanoseconds() {

	uint32_t count = 0;
	int64_t returnTime = 0;

	// -> current time = nanoTime + 1 000 000 000 * countRegister/tim2Clock

	// Disable Interrupt is no solution here (wrong values caused by missed interrupt)

	// Read nanoTime twice, to make sure it has not changed while reading counter value
	do {
		returnTime = nanoTime;
		count = (TIMx)->Instance->CNT;
	} while(returnTime != nanoTime);


	/** low precision
	 * - nanos = 166ns * count (for tim2Clock = 6MHz)
	 * - nanos = 11ns * count (for tim2Clock = 84MHz)
	 * ! startup time might be wrong due to integer overflow !
	 */
	//int nanos = 1000000000/timerClock * count;

	/** high precision
	 * - nanos = 166,666666ns * count (for tim2Clock = 6MHz)
	 * - nanos = 11,904761s * count (for tim2Clock = 84MHz)
	 * - takes 4 times longer than low precision
	 */
	long long nanos = 0;
	nanos = 1000000000/(timerClock/1000000);
	nanos *= count;
	nanos /= 1000000;

	return returnTime + nanos;
}


void hwInitTime()
{
	nanoTime=0;
}


int64_t hwGetAbsoluteNanoseconds()
{
	return hwGetNanoseconds();// + timeAtStartup;
}

#ifndef NO_RODOS_NAMESPACE
}
#endif
