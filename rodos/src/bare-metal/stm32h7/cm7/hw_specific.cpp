#include "hw_specific.h"
#include "stm32h7xx.h"
#include "thread.h"

namespace RODOS {
/**
 * hwInitContext()
 * Builds an initial exception-return stack frame for a new thread.
 *
 * Cortex-M7 hardware automatically stacks/unstacks xPSR, PC, LR, R12, R3-R0
 * on exception entry/exit. We fake that frame so the first "return from ISR"
 * lands in threadStartupWrapper(object).
 *
 *  Stack layout built here (top = lowest address, grows downward):
 *  [ EXC_RETURN sentinel ] <- returned as initial SP
 *  [ r4 – r11            ]   (callee-saved, zeroed)
 *  ---- hardware auto-stacked frame (8 words) ----
 *  [ r0  = object        ]
 *  [ r1  = 0             ]
 *  [ r2  = 0             ]
 *  [ r3  = 0             ]
 *  [ r12 = 0             ]
 *  [ LR  = 0xFFFFFFFD    ]  EXC_RETURN: return to Thread mode, use PSP
 *  [ PC  = threadStartupWrapper ]
 *  [ xPSR = 0x01000000   ]  Thumb bit set
 */
long* hwInitContext(long* stack, void* object) {
    // Stack pointer starts at top of allocated region; move down for alignment
    stack--;   // padding to keep 8-byte alignment after the frame

    // --- Hardware exception frame (stacked by CPU on interrupt entry) ---
    *stack-- = 0x01000000UL;                        // xPSR  (Thumb bit)
    *stack-- = (long)threadStartupWrapper;          // PC
    *stack-- = 0xFFFFFFFDUL;                        // LR    (EXC_RETURN: PSP, Thread)
    *stack-- = 0;                                   // R12
    *stack-- = 0;                                   // R3
    *stack-- = 0;                                   // R2
    *stack-- = 0;                                   // R1
    *stack-- = (long)object;                        // R0    (1st argument)

    // --- Software-saved callee registers R4–R11 ---
    for (int i = 0; i < 8; i++) {
        *stack-- = 0;
    }

    // Return pointer to current top-of-stack (what SP will be restored to)
    return stack + 1;
}
}

extern "C" {

/**
 * hwInit()
 * Initializes core STM32H747 hardware via HAL:
 * - System clock, SysTick, FPU, cache.
 */
void hwInit() {
    HAL_Init();               // Resets peripherals, initialises SysTick (1 ms tick)
    SystemClock_Config();     // Must be provided by your CubeMX/board init code

    // Enable FPU for Cortex-M7 (full access, CP10 & CP11)
    SCB->CPACR |= (0xF << 20);
    __DSB();
    __ISB();

    // Enable I-Cache and D-Cache for performance
    SCB_EnableICache();
    SCB_EnableDCache();
}



void hwResetAndReboot() {
    NVIC_SystemReset();   // Cortex-M CMSIS call
}

void sp_partition_yield() { }   // bare-metal stub
void startIdleThread()    { }   // bare-metal stub

} // extern "C"
