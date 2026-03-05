

/**
* @file scheduler.cc
* @date 2008/04/24 18:10
* @author Lutz Dittrich
*
* @brief priority based scheduler
*
*/
#include "scheduler.h"

#include "rodos.h"
#include "rodos-atomic.h"
#include "hw_specific.h"

namespace RODOS {

extern "C" {
  /**
  * A piece of memory used as stack by the scheduler.
  */
  char __schedulerStack__[SCHEDULER_STACKSIZE];

  /**
  * Address of the top of the scheduler's stack.
  * 8-byte aligned
  */
  long __schedulerStackFrame__ =
                  ((long)&(__schedulerStack__[SCHEDULER_STACKSIZE-sizeof(long)])) & (~7);
}

/** count all calls to the scheduler */
RODOS::AtomicRO<uint64_t> Scheduler::scheduleCounter{0};

Thread* Scheduler::preSelectedNextToRun = 0;
int64_t Scheduler::preSelectedEarliestSuspendedUntil = END_OF_TIME;

extern Thread* idlethreadP;
extern RODOS::Atomic<int64_t> timeToTryAgainToSchedule;


void schedulerWrapper(long* ctx) {
    Thread *active_trd = Thread::currentThread.loadFromISR();
    active_trd->context = ctx;

#ifndef DISABLE_TIMEEVENTS
    TimeEvent::propagate(NOW());
#endif

    int64_t nextSchedulingEventTime = timeToTryAgainToSchedule.loadFromISR();
    if (NOW() < nextSchedulingEventTime) {
        // it's not the time to schedule (SysTick only triggered for TimeEvent)

#ifndef DISABLE_TIMEEVENTS
        int64_t nextTimeEventTime = TimeEvent::getNextTriggerTime();
#else
        int64_t nextTimeEventTime = END_OF_TIME;
#endif

        Timer::updateTriggerToNextTimingEvent(nextSchedulingEventTime, nextTimeEventTime);
        Timer::start();
    } else {
        active_trd = Scheduler::schedule();
    }

    // resume active thread
    active_trd->activate();
}

/** activate idle thread */
void Scheduler::idle() {
    idlethreadP->suspendedUntil.store(0);

    Thread::currentThread.store(idlethreadP);
    schedulerRunning = true;  /* a bit to early, but no later place possible */

    int64_t nextSchedulingEventTime = timeToTryAgainToSchedule.load();
    int64_t nextTimeEventTime = END_OF_TIME;
#ifndef DISABLE_TIMEEVENTS
    nextTimeEventTime = TimeEvent::getNextTriggerTime();
#endif
    Timer::updateTriggerToNextTimingEvent(nextSchedulingEventTime, nextTimeEventTime);

    /*
     * - the order of activate() and startIdleThread() is important -> don't change
     * - for all cortex ports a global context pointer is initialised in activate()
     *   and this must have been done before startIdleThread() is called.
     */
    idlethreadP->activate();
    startIdleThread(); // only for some architectures, most implementations == nop()
}


Thread* Scheduler::schedule() {
    // increment the schedule counter
    scheduleCounter.storeFromISR(scheduleCounter.loadFromISR() + 1);

    // time events to call?
    // now obsolete! call directly from timer!! TimeEvent::propagate(timeNow);

    // select the next thread to run: Do we have a preselection from Thread::yield()?
    Thread* nextThreadToRun = Scheduler::preSelectedNextToRun; // eventually set by Thread::yield()
    int64_t selectedEarliestSuspendedUntil = Scheduler::preSelectedEarliestSuspendedUntil;

    // in case we don't already have a preselected thread to run
    // -> actually do the work of finding the next thread in the schedule
    if (nextThreadToRun == nullptr) {
        nextThreadToRun = Thread::findNextToRunFromISR(selectedEarliestSuspendedUntil);
    }
    // use preselection only once
    Scheduler::preSelectedNextToRun = nullptr;

    // check if selected thread has stack violations (if yes -> switch to idleThread)
    if (nextThreadToRun->checkStackViolations()) {
        nextThreadToRun = idlethreadP;
    }

    // update the respective variables according to the schedule
    nextThreadToRun->lastActivation.storeFromISR(Scheduler::scheduleCounter.loadFromISR()); // timeNow ?? but what with on-os_xx, on-posix, etc?

    // it is important to set timeToTryAgainToSchedule as late as possible,
    // because we don't want to (potentially) steal time from our thread's next time slice
    // -> if the thread gets the whole time slice and updating the next SysTick succeeds,
    //    we only "waste" the thread's time for a small constant time period
    int64_t nextTimeEventTime = END_OF_TIME;
#ifndef DISABLE_TIMEEVENTS
    nextTimeEventTime = TimeEvent::getNextTriggerTime();
#endif

    int64_t nextSchedulingEventTime = min(selectedEarliestSuspendedUntil, TIME_SLICE_FOR_SAME_PRIORITY + NOW());
    timeToTryAgainToSchedule.storeFromISR(nextSchedulingEventTime);

    // update SysTick timer to next event
    Timer::updateTriggerToNextTimingEvent(nextSchedulingEventTime, nextTimeEventTime);
    return nextThreadToRun;
}

uint64_t Scheduler::getScheduleCounter() {
    return scheduleCounter.load();
}

} /* namespace RODOS */
