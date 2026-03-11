#include "rodos.h"
#include "stm32h7xx.h"

namespace RODOS {


extern "C" {
static TIM_HandleTypeDef htim2;
static volatile uint32_t timerOverflowCount = 0; // counts TIM2 wrap-arounds

/**
 * hwInitTime()
 * Configures a free-running 32-bit hardware timer (TIM2 on STM32H747)
 * clocked at 1 MHz → 1 tick = 1 µs. A 64-bit software counter extends
 * the range to centuries.
 *
 * TIM2 on H747 sits on APB1 (max 240 MHz). We prescale to exactly 1 MHz.
 */
void hwInitTime() {
    __HAL_RCC_TIM2_CLK_ENABLE();

    // APB1 timer clock = 240 MHz on a typical H747 config
    // Prescaler: 240 - 1  →  1 MHz tick (1 tick = 1 µs)
    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = (SystemCoreClock / 2 / 1000000) - 1;
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 0xFFFFFFFF; // full 32-bit range
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);

    // Enable update (overflow) interrupt to track 64-bit time
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_Base_Start(&htim2);
}

/**
 * hwGetNanoseconds()
 * Combines the 32-bit hardware counter with the overflow counter
 * into a 64-bit microsecond timestamp, then converts to nanoseconds.
 *
 * Guard against timer overflow racing between reading the two values
 * by double-checking the overflow count.
 */
long long unsigned int hwGetNanoseconds() {
    uint32_t overflow1, ticks;

    // Re-read if an overflow happened between the two reads
    do {
        overflow1 = timerOverflowCount;
        ticks     = TIM2->CNT;
    } while(overflow1 != timerOverflowCount);

    uint64_t microseconds = ((uint64_t)overflow1 << 32) | ticks;
    return microseconds * 1000ULL; // µs → ns
}

// ISR: increments overflow counter every ~4295 seconds (2^32 µs)
void TIM2_IRQHandler() {
    if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE)) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        timerOverflowCount += 1;
    }
}
} // extern "C"
} // namespace RODOS
