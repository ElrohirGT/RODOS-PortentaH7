#include <hw_specific.h>
#include <stm32h747xx.h>
#include <thread.h>

volatile long* contextT;

namespace RODOS {

extern "C" {
void __asmSwitchToContext(long* context) { contextT = context; }

void __asmSaveContextAndCallScheduler() {
    /* Set a PendSV-interrupt to request a context switch. */
    SCB->ICSR = SCB->ICSR | SCB_ICSR_PENDSVSET_Msk;
}
}

uintptr_t Thread::getCurrentStackAddr() { return reinterpret_cast<uintptr_t>(context.load()); }

void enterSleepMode() { __WFI(); }

void hwDisableInterrupts() { __disable_irq(); }

void hwEnableInterrupts() { __enable_irq(); }
} // namespace RODOS
