#include "rodos.h"
#include "stm32h7xx.h"

namespace RODOS {


static TIM_HandleTypeDef htim5;
static volatile uint32_t timerOverflowCount = 0;

/**
 * Timer choice for CM4: TIM5 (also 32-bit, on APB1).
 * We avoid TIM2 here to prevent conflicts if CM7 uses it simultaneously.
 * Both cores share the same peripheral bus — coordinate ownership carefully.
 *
 * APB1 timer clock on CM4 = 120 MHz (half of CM4 max 240 MHz by default).
 * Prescaler target: 1 MHz (1 tick = 1 µs), same as CM7 side.
 */
void hwInitTime() {
    __HAL_RCC_TIM5_CLK_ENABLE();

    // APB1 timer clock = SystemCoreClock / 2 on default H747 clock tree
    htim5.Instance               = TIM5;
    htim5.Init.Prescaler         = (SystemCoreClock / 2 / 1000000) - 1;
    htim5.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim5.Init.Period            = 0xFFFFFFFF; // full 32-bit range
    htim5.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim5);

    __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_UPDATE);
    HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);

    HAL_TIM_Base_Start(&htim5);
}

void TIM5_IRQHandler() {
    if(__HAL_TIM_GET_FLAG(&htim5, TIM_FLAG_UPDATE)) {
        __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_UPDATE);
        timerOverflowCount += 1;
    }
}

long long unsigned int hwGetNanoseconds() {
    uint32_t overflow1, ticks;

    do {
        overflow1 = timerOverflowCount;
        ticks     = TIM5->CNT;
    } while(overflow1 != timerOverflowCount);

    uint64_t microseconds = ((uint64_t)overflow1 << 32) | ticks;
    return microseconds * 1000ULL;
}
} // namespace RODOS
