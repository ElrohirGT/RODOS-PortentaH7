#include <hw_specific.h>
#include <stm32h7xx.h>
#include <thread.h>
#include <h7_modules.h>

namespace RODOS {
/* Constants required to set up the initial stack. */
constexpr long INITIAL_XPSR        = 0x01000000l;
constexpr long INITIAL_EXEC_RETURN = static_cast<long>(0xfffffffdl);

/**
 * hwInitContext()
 * Identical stack frame layout to CM7 — both are ARMv7-M Thumb-2.
 * The Cortex-M exception frame format is the same on CM4 and CM7.
 */
long* hwInitContext(long* stack, void* object) {
    stack--; // padding for 8-byte alignment

    // Hardware exception frame
    *stack-- = INITIAL_XPSR;                 // xPSR  (Thumb bit set)
                                             // *stack-- = (long)(threadStartupWrapper);  // PC
    *stack-- = (long)(threadStartupWrapper); // PC
    *stack-- = INITIAL_EXEC_RETURN;          // LR    (EXC_RETURN: PSP, Thread, no FP)
    *stack-- = 0;                            // R12
    *stack-- = 0;                            // R3
    *stack-- = 0;                            // R2
    *stack-- = 0;                            // R1
    *stack-- = (long)object;                 // R0    (threadStartupWrapper argument)

    // Software-saved callee registers R4–R11
    for(int i = 0; i < 8; i++) { *stack-- = 0; }

    return stack + 1;
}

void sp_partition_yield() {}
void startIdleThread() {}
} // namespace RODOS

extern "C" {

/**
 * hwInit()
 * CM4-side hardware init. Much simpler than CM7:
 * - No cache management (CM4 has no I/D cache)
 * - FPU enable is still required
 * - HAL_Init() re-initialises SysTick for this core
 * - Clock config on CM4 is read-only: CM7 already set up the PLLs.
 *   We only need to update SystemCoreClock for correct HAL tick math.
 */
void hwInit() {
    __HAL_RCC_HSEM_CLK_ENABLE();

    // Update the SystemCoreClock variable.
    SystemCoreClockUpdate();
    HAL_Init();

    // Initialize modules
    MX_GPIO_Init();
    MX_UART_Init();
}


void hwResetAndReboot() { NVIC_SystemReset(); }

} // extern "C"
