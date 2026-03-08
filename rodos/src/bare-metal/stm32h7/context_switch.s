.syntax unified
    .thumb
    .text

/**
 * void __asmSwitchToContext(long* context)
 *
 * R0 = pointer to saved context (top of target thread's stack).
 * Restores R4-R11 from the software frame, then sets PSP and
 * performs an EXC_RETURN so the CPU unstacks R0-R3, R12, LR, PC, xPSR.
 */
    .global __asmSwitchToContext
    .thumb_func
__asmSwitchToContext:
    /* Restore callee-saved registers from software frame */
    LDMIA   R0!, {R4-R11}

    /* Set Process Stack Pointer to remainder of frame */
    MSR     PSP, R0

    /* Ensure writes are visible before returning */
    ISB

    /* EXC_RETURN value: return to Thread mode using PSP, no FP state */
    LDR     LR, =0xFFFFFFFD
    BX      LR


/**
 * void __asmSaveContextAndCallScheduler()
 *
 * Called (typically from SysTick/PendSV ISR) to:
 *  1. Save the current thread's callee-saved registers onto its PSP stack.
 *  2. Store the updated SP into the RODOS thread object.
 *  3. Call the RODOS scheduler (scheduler()).
 *  4. Load the SP of the next thread and switch to it.
 *
 * RODOS convention: the global pointer `getCurrentThread()->context`
 * points to the stored SP slot for the running thread.
 */
    .global __asmSaveContextAndCallScheduler
    .extern getCurrentThread          /* RODOS kernel call */
    .extern scheduler                 /* RODOS scheduler   */
    .thumb_func
__asmSaveContextAndCallScheduler:
    /* 1. Read current PSP (the interrupted thread's SP) */
    MRS     R0, PSP
    ISB

    /* 2. Push callee-saved registers onto that stack */
    STMDB   R0!, {R4-R11}

    /* 3. Save updated SP → thread->context  */
    PUSH    {LR}                  /* preserve EXC_RETURN across C calls  */
    BL      getCurrentThread      /* R0 = pointer to current Thread obj  */
    /*   Thread::context is the first field (offset 0) per RODOS layout  */
    MRS     R1, PSP
    STMDB   R1!, {R4-R11}        /* already done above; recalc cleanly   */
    STR     R1, [R0]             /* thread->context = new SP             */

    /* 4. Run the scheduler — it updates the global "next thread" */
    BL      scheduler

    /* 5. Load next thread's SP and switch */
    BL      getCurrentThread      /* R0 = next Thread (scheduler updated it) */
    LDR     R0, [R0]             /* R0 = next thread->context (saved SP)  */
    LDMIA   R0!, {R4-R11}        /* restore callee-saved registers        */
    MSR     PSP, R0
    ISB
    POP     {LR}                 /* restore EXC_RETURN value              */
    BX      LR                   /* return from exception → new thread    */

    .end
