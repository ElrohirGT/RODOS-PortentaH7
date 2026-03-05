

/**
* @file thread.cc
* @date 2008/04/22 16:50
* @author Lutz Dittrich, Sergio Montenegro
*
*
* A Thread is a schedulable object with own context and stack
*
* @brief %Thread handling
*/
#include "thread.h"

#include "rodos.h"
#include "rodos-atomic.h"
#include "scheduler.h"
#include "hw_specific.h"
#include "platform-parameter.h"

namespace RODOS {

constexpr uint32_t EMPTY_MEMORY_MARKER = 0xDEADBEEF;

RODOS::Atomic<int64_t> timeToTryAgainToSchedule{0};
RODOS::Atomic<bool> yieldSchedulingLock{false};

/** old style constructor */
Thread::Thread(const char* name,
               const int32_t priority,
               const size_t _stackSize) :
    ListElement(threadList, name) {

    this->stackSize = _stackSize;
    stackBegin = static_cast<char*>(xmalloc(stackSize));
    stack = reinterpret_cast<long*>((reinterpret_cast<uintptr_t>(stackBegin) + (stackSize-4)) & (~static_cast<uintptr_t>(7u))); // align 8 byte
    this->priority = priority;

    //Paint the stack space; TODO: Comment out for faster start up
    initializeStack();
}

void Thread::initializeStack() {
    //Paint the stack space TODO: Comment out for faster start up
    uint32_t* stackPaint = (uint32_t*)stack;
    while((intptr_t)stackPaint >= (intptr_t)stackBegin) {
        *stackPaint = EMPTY_MEMORY_MARKER;
        stackPaint--;
    }

    context = hwInitContext(stack, this);
}

bool Thread::checkStackViolations() {
    /** Check stack violations **/
    constexpr size_t stackMargin      = 300;
    uintptr_t        minimumStackAddr = reinterpret_cast<uintptr_t>(this->stackBegin) + stackMargin;
    if(this->getCurrentStackAddr() < minimumStackAddr) {
        xprintf("!StackOverflow! %s DEACTIVATED!: free %d\n",
                this->name,
                static_cast<int>(this->getCurrentStackAddr() - reinterpret_cast<uintptr_t>(this->stackBegin)));
        this->suspendedUntil.store(END_OF_TIME);
        return true;
    }
    if(*reinterpret_cast<uint32_t*>(this->stackBegin) != EMPTY_MEMORY_MARKER) { // this thread is going beyond its stack!
        xprintf("! PANIC %s beyond stack, DEACTIVATED!\n", this->name);
        this->suspendedUntil.store(END_OF_TIME);
        return true;
    }

    return false;
}

Thread::~Thread() {
    if(isShuttingDown) return;
    PRINTF("%s:",getName());
    RODOS_ERROR("Thread deleted");
}

/* called in main() after all constuctors, to create/init thread */
void Thread::create() {
    // only required when implementig in on the top of posix, rtems, freertos, etc
}


/** pause execution of this thread and call scheduler */
void Thread::yield() {
    // we want to perform an unplanned schedule => reset precalculated time
    timeToTryAgainToSchedule.store(0);
    // atomically save scheduleCounter to measure if scheduler is triggered during yield
    uint64_t startScheduleCounter = Scheduler::getScheduleCounter();

    // Optimisation: Avoid unnecessary context switch: see Scheduler::schedule()
    int64_t selectedEarliestSuspendedUntil = END_OF_TIME;
    Thread* preselection                   = findNextToRun(selectedEarliestSuspendedUntil);

    // if scheduler triggered during find, preselection is invalid
    // -> simultaneous scheduler call => already yielded (can just return)
    uint64_t currentScheduleCounter = Scheduler::getScheduleCounter();
    if(startScheduleCounter != currentScheduleCounter) {
        return;
    }
    // from here on we know that preselection is valid (there was no simultaneous scheduling event)

    // If scheduler would choose same thread, return directly (no other thread wants to take over).
    // Cases regarding simultaneous scheduling events (since last if):
    // 1) no simultaneous scheduling event:
    //    -> can directly return as no other thread wants to run + no context switch (optimisation)
    //    -> no computation lease renewal for thread, but that is not required from yield
    // 2) simultaneous scheduling event(s) to at least one other thread:
    //    -> other thread(s) already got scheduled during yield (no manual scheduler call needed)
    //    -> when this thread gets scheduled again, we can directly return
    // 3) simultaneous scheduling event to same thread:
    //    -> no other thread wanted to run, computation lease just got renewed for this thread
    //    -> no need to call the scheduler again
    if(preselection == getCurrentThread()) {
        return;
    }
    // from here on we know that there is (was) a thread wanting to run (we must call scheduler)

    // we have to stop Timer (as it might be unsafe to call scheduler from thread otherwise)
    // and after Timer stop no simultaneous scheduling events are triggered anymore
    // -> Timer::stop() is non-atomic, so must abort simultaneous SysTicks via yieldSchedulingLock
    //    and we must keep the lock until thread activate as some ports don't implement Timer stop
    yieldSchedulingLock = true;
    Timer::stop();

    // Cases regarding simultaneous scheduling events (between last if and Timer stop):
    // 1) no simultaneous scheduling event:
    //    -> just call scheduler as other thread is waiting for us
    //    -> we can even reuse our precomputed scheduling parameters (optimisation)
    // 2) simultaneous scheduling event(s) to at least one other thread:
    //    -> we have already switched to other thread(s) and now we have been scheduled again
    //    -> in theory not necessary to still call scheduler, but we must now as we stopped Timer
    // 3) simultaneous scheduling event to same thread:
    //    -> only possible when also simultaneous scheduling event(s) to other thread (see above)
    if(startScheduleCounter == Scheduler::getScheduleCounter()) {
        Scheduler::preSelectedNextToRun              = preselection;
        Scheduler::preSelectedEarliestSuspendedUntil = selectedEarliestSuspendedUntil;
    }
    __asmSaveContextAndCallScheduler();
}

/* restore context of this thread and continue execution of this thread */
void Thread::activate() {
    currentThread = this;
    // release yieldSchedulingLock before starting SysTicks again in case we are called from yield
    // -> this is done so late because some ports don't implement Timer::stop
    yieldSchedulingLock = false;
    Timer::start();
    __asmSwitchToContext((long*)context);
}


/*******************************************************************/

/* get priority of the thread */
int32_t Thread::getPriority() const {
    return priority;
}

/* set priority of the thread */
void Thread::setPriority(const int32_t prio) {
    priority = prio;
}

Thread* Thread::getCurrentThread() {
    return currentThread;
}



/* resume the thread */
void Thread::resume() {
    timeToTryAgainToSchedule = 0;
    waitingFor     = nullptr;
    suspendedUntil = 0;
    // yield(); // commented out because resume may be called from an interrupt server
    // maybe use __asmSaveContextAndCallScheduler():
    //  (+) more responsive, if a high-priority thread is resumed
    //  (-) "steals" time due to rescheduling, if a low-priority thread is resumed
}

/* suspend the thread */
bool Thread::suspendCallerUntil(const int64_t reactivationTime, void* signaler) {

    Thread* caller =  getCurrentThread();
    {
        PRIORITY_CEILER_IN_SCOPE();
        caller->waitingFor = signaler;
        caller->suspendedUntil = reactivationTime;
    }
    yield();

    caller->waitingFor = nullptr;
    /** after yield: It was resumed (suspendedUntil set to 0) or time was reached ?*/
    if(caller->suspendedUntil == 0) return true; // it was resumed!
    return false; // time was reached
}



void Thread::initializeThreads() {
    xprintf("Threads in System:");
    ITERATE_LIST(Thread, threadList) {
        xprintf("\n   Prio = %7ld Stack = %6lu %s: ",
            static_cast<long>(iter->priority),
            static_cast<unsigned long>(iter->stackSize),
            iter->getName());
        iter->init();
        iter->suspendedUntil = 0;
    }
    xprintf("\n");
    ITERATE_LIST(Thread, threadList) {
        iter->create();
    }
}

// not used in this implementation, the scheduler activates thread
void Thread::startAllThreads() { }


/** non-static C++ member functions cannot be used like normal
   C function pointers. www.function-pointer.org suggests using a
   wrapper function instead. */

void threadStartupWrapper(Thread* thread) {
    Thread::currentThread = thread;
    thread->suspendedUntil = 0;

    thread->run();
    /*
      loop forever
      if run() returns this thread is to be considered terminated
    */

    while(1) {
        thread->suspendedUntil = END_OF_TIME;
        thread->yield();
    }
}


unsigned long long Thread::getScheduleCounter() {
    return Scheduler::getScheduleCounter();
}

/********************************************************************************/


/**
 * @class IdleThread
 * @brief The idle thread.
 *
 * The idle thread. This thread will be executed if no other thread wants to
 * run
 */
class IdleThread : public StaticThread<> {
public:
    IdleThread() : StaticThread("IdleThread", 0) {
    }
    void run();
    void init();
};

void IdleThread::run() {
    while(1) {
        idleCnt = idleCnt + 1;
        setPriority(0); // Due to wrong usage of PRIORITY_CLING in events, once I got highest prio for Idle.
        sp_partition_yield(); // allow other linux processes or ARIC-653 partitions to run
        yield();

#ifndef DISABLE_SLEEP_WHEN_IDLE
        // enter sleep mode if suitable
        int64_t reactivationTime = timeToTryAgainToSchedule;
#ifndef DISABLE_TIMEEVENTS
        reactivationTime =
            RODOS::min(timeToTryAgainToSchedule.load(), TimeEvent::getNextTriggerTime());
#endif

        int64_t durationToNextTimingEvent = reactivationTime - NOW();
        int64_t timerInterval =
            durationToNextTimingEvent - TIME_WAKEUP_FROM_SLEEP - MIN_SYS_TICK_SPACING;
        if(timerInterval > TIME_WAKEUP_FROM_SLEEP && timerInterval > MIN_SYS_TICK_SPACING) {
            Timer::stop();
            Timer::setInterval(timerInterval / 1000l); // nanoseconds to microseconds
            Timer::start();

            enterSleepMode();

            Timer::stop();
            auto remainingTime = RODOS::max(reactivationTime - NOW(), MIN_SYS_TICK_SPACING);
            Timer::setInterval(remainingTime / 1000l); // nanoseconds to microseconds
            Timer::start();
        }
#endif
    }
}

void IdleThread::init() {
    xprintf("yields all the time");
}


/**
 * The idle thread.
 */
IdleThread idlethread;
Thread* idlethreadP = &idlethread;


/*********************************************************************************/

constexpr int64_t earlier(const int64_t a, const int64_t b) {
    return (a < b) ? a : b;
}

Thread* Thread::findNextToRun(int64_t& selectedEarliestSuspendedUntil) {
    Thread* nextThreadToRun = &idlethread; // Default, if no one else wants
    selectedEarliestSuspendedUntil = END_OF_TIME;
    int64_t timeNow = NOW();

    ITERATE_LIST(Thread, threadList) {
        // only load suspendedUntil once, as this may be changed by interrupts during scheduling
        int64_t iterSuspendedUntil = iter->suspendedUntil.load();
        int32_t iterPrio = iter->getPriority();
        int32_t nextThreadToRunPrio = nextThreadToRun->getPriority();
        if (iterSuspendedUntil < timeNow) { // in the past
			// - thread with highest prio will be executed immediately when this scheduler-call ends
            // - other threads with lower prio will be executed after next scheduler-call
            //   due to suspend() of high-prio thread
            if (iterPrio > nextThreadToRunPrio) {
                nextThreadToRun = iter;
            }
            else if (iterPrio == nextThreadToRunPrio) {
                if (iter->lastActivation.load() < nextThreadToRun->lastActivation.load()) {
                    nextThreadToRun = iter;
                }
            }

        } else { // in the future, find next to be handled
			// if there is a thread with higher or same priority in the future, we must call the scheduler then
			// so that the thread will be executed
            if(iterPrio >= nextThreadToRunPrio) {
                selectedEarliestSuspendedUntil =
                    earlier(selectedEarliestSuspendedUntil, iterSuspendedUntil);
            }
			// threads with lower priority will not be executed until nextThreadToRun suspends
        }
    } // Iterate list

    return nextThreadToRun;
}

// It's completely identical to the above function except all "load()" calls
// are replaced with "loadFromISR()" ones (when accessing RODOS::Atomics)
Thread* Thread::findNextToRunFromISR(int64_t& selectedEarliestSuspendedUntil) {
    Thread* nextThreadToRun = &idlethread; // Default, if no one else wants
    selectedEarliestSuspendedUntil = END_OF_TIME;
    int64_t timeNow = NOW();

    ITERATE_LIST(Thread, threadList) {
        // only load suspendedUntil once, as this may be changed by interrupts during scheduling
        int64_t iterSuspendedUntil = iter->suspendedUntil.loadFromISR();
        int32_t iterPrio = iter->priority.loadFromISR();
        int32_t nextThreadToRunPrio = nextThreadToRun->priority.loadFromISR();
        if (iterSuspendedUntil < timeNow) { // in the past
            // - thread with highest prio will be executed immediately when this scheduler-call ends
            // - other threads with lower prio will be executed after next scheduler-call
            //   due to suspend() of high-prio thread
            if (iterPrio > nextThreadToRunPrio) {
                nextThreadToRun = iter;
            }
            else if (iterPrio == nextThreadToRunPrio) {
                if (iter->lastActivation.loadFromISR() < nextThreadToRun->lastActivation.loadFromISR()) {
                    nextThreadToRun = iter;
                }
            }
        } else { // in the future, find next to be handled
            // if there is a thread with higher or same priority in the future, we must call the scheduler then
            // so that the thread will be executed
            if(iterPrio >= nextThreadToRunPrio) {
                selectedEarliestSuspendedUntil =
                    earlier(selectedEarliestSuspendedUntil, iterSuspendedUntil);
            }
            // threads with lower priority will not be executed until nextThreadToRun suspends
        }
    } // Iterate list

    return nextThreadToRun;
}


Thread* Thread::findNextWaitingFor(void* signaler) {
    Thread* nextWaiter = &idlethread; // Default, if no one else wants to run

    ITERATE_LIST(Thread, threadList) {
        if (iter->waitingFor == signaler) {
            if (iter->getPriority() > nextWaiter->getPriority()) {
                nextWaiter = iter;
            } else {
                if (iter->getPriority() == nextWaiter->getPriority()) {
                    if (iter->lastActivation < nextWaiter->lastActivation) {
                        nextWaiter = iter;
                    }
                }
            }
        }
    }
    if (nextWaiter == &idlethread) {
        return nullptr;
    }
    return nextWaiter;
}

size_t Thread::getMaxStackUsage(){
	Thread* currentThread = getCurrentThread();

	//Go to the beginning of the stack(lowest addres)
	uint32_t* stackScan = (uint32_t*)currentThread->stack;
	while((intptr_t)stackScan >= (intptr_t)currentThread->stackBegin){
		stackScan--;
	}
	stackScan++;

	//Go up until empty markers are found and count
	size_t freeStack=0;
	while(stackScan <= (uint32_t*)currentThread->stack && *stackScan == EMPTY_MEMORY_MARKER){
		freeStack +=4;
		stackScan++;
	}

	return currentThread->stackSize-freeStack;
}



} // namespace

