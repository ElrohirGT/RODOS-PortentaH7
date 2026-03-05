#include "rodos.h"

uint32_t printfMask = 0;

/****** Use moduleId 2000 just be be found by other example: ceiler **/
static Application module01("PreemptionTest", 2000);

class HighPriorityThread : public StaticThread<> {
  public:
    static constexpr auto PRINT_INTERVAL = 500 * MILLISECONDS;

    HighPriorityThread() : StaticThread<>("HiPriority", 25) {
    }

    void init() {
        printfMask = 1;
        xprintf(" hipri  = '*'\n");
        printfMask = 0;
    }

    void run() {
        printfMask = 1;
        TIME_LOOP(0, PRINT_INTERVAL) {
            xprintf("*");
            FFLUSH();
        }
    }
};


class LowPriorityThread : public StaticThread<> {
  public:
    static constexpr auto PRINT_INTERVAL   = 50 * MILLISECONDS;
    static constexpr auto PRINTF_WAIT_TIME = 5 * MILLISECONDS;
    static constexpr auto WAIT_JITTER      = 2 * MILLISECONDS;

    LowPriorityThread() : StaticThread<>("LowPriority", 10) {
    }

    void init() {
        printfMask = 1;
        xprintf(" lopri  = '.'\n");
        printfMask = 0;
    }

    void run() {
        printfMask = 1;

        auto nextTick = TimeModel::computeNextBeat(PRINTF_WAIT_TIME, PRINT_INTERVAL, NOW());
        while(true) {
            if((nextTick <= NOW()) && (NOW() <= nextTick + WAIT_JITTER)) {
                xprintf(".");
                FFLUSH();
            }
            nextTick = TimeModel::computeNextBeat(PRINTF_WAIT_TIME, PRINT_INTERVAL, NOW());
            busyWaitUntil(nextTick);
        }
    }
};


/******************/

static Application module02("PriorityCeiling", 3000);

class PriorityCeilerThread : public StaticThread<> {
  public:
    static constexpr auto PRINT_INTERVAL   = 50 * MILLISECONDS;
    static constexpr auto PRINTF_WAIT_TIME = 10 * MILLISECONDS;
    static constexpr auto WAIT_JITTER      = 2 * MILLISECONDS;

    static constexpr auto INIT_WAIT_TIME = 3 * SECONDS;

    static constexpr auto START_LOOP_1          = INIT_WAIT_TIME;
    static constexpr auto DURATION_LOOP_1       = 5 * SECONDS;
    static constexpr auto START_LOOP_2          = START_LOOP_1 + DURATION_LOOP_1;
    static constexpr auto DURATION_LOOP_2       = 3 * SECONDS;
    static constexpr auto START_LOOP_3          = START_LOOP_2 + DURATION_LOOP_2;
    static constexpr auto DURATION_LOOP_3       = 5 * SECONDS;
    static constexpr auto START_LOOP_4          = START_LOOP_3 + DURATION_LOOP_3;
    static constexpr auto DURATION_LOOP_4       = 3 * SECONDS;
    static constexpr auto START_LAST_SUSPEND    = START_LOOP_4 + DURATION_LOOP_4;
    static constexpr auto DURATION_LAST_SUSPEND = 2 * SECONDS;

    PriorityCeilerThread() : StaticThread<>("PriorityCeilerThread", 50) {
    }

    void init() {
        printfMask = 1;
        xprintf(" ceiler = '-'\n");
        printfMask = 0;
    }


    void run() {
        printfMask = 1;
        xprintf("PriorityCeilerThread: waits for %llds\n", INIT_WAIT_TIME / SECONDS);
        this->setPriority(10);

        // add WAIT_JITTER to suspending to allow last PRINTF in other thread to finish
        suspendCallerUntil(INIT_WAIT_TIME + WAIT_JITTER);

        /*
         * PriorityCeiler normal schedule (time diagram sketch)
         * -> PriorityCeilerThread and LowPriorityThread print periods are shifted a bit
         *    (by respective PRINTF_WAIT_TIME)
         *
         * HighPriorityThread: P-------------------------------------------------P------
         * LowPriorityThread:  -PxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxP
         * PriorityCeiler:     --PxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxxPxxxx
         * threads scheduled:  HCCCCCCCCCCLLLLLLLLLLCCCCCCCCCCLLLLLLLLLLCCCCCCCCCHLLLLLL
         * threads printing:   H C    C   L    L     C    C   L    L     C    C  HL    L
         *
         * -> 'P': thread prints
         * -> 'x': thread is busy waiting
         * -> '-': thread is suspending
         * -> 'H': HighPriorityThread is scheduled/prints
         * -> 'L': LowPriorityThread is scheduled/prints
         * -> 'C': PriorityCeilerThread is scheduled/prints
         */
        xprintf("\n"
                "PriorityCeilerThread: %llds normal\n",
                DURATION_LOOP_1 / SECONDS);
        busyWaitLoop(START_LOOP_1, DURATION_LOOP_1);

        xprintf("\n"
                "PriorityCeilerThread: %llds with PRIORITY_CEILER_IN_SCOPE\n",
                DURATION_LOOP_2 / SECONDS);
        {
            PRIORITY_CEILER_IN_SCOPE();

            busyWaitLoop(START_LOOP_2, DURATION_LOOP_2 - PRINT_INTERVAL);
            xprintf("\n"
                    "PriorityCeilerThread: yielding -> "
                    "HighPriorityThread prints twice to make up for missed PRINTFs\n");
        }
        yield();

        xprintf("\n"
                "PriorityCeilerThread: %llds normal\n",
                DURATION_LOOP_3 / SECONDS);
        busyWaitLoop(START_LOOP_3, DURATION_LOOP_3);

        xprintf("\n"
                "PriorityCeilerThread: %llds normal + suspending for %llds\n",
                DURATION_LOOP_4 / SECONDS,
                DURATION_LAST_SUSPEND / SECONDS);
        busyWaitLoop(START_LOOP_4, DURATION_LOOP_4);

        suspendCallerUntil(START_LAST_SUSPEND + DURATION_LAST_SUSPEND);

        xprintf("\nTest terminates\n");
        hwResetAndReboot();
    }

  private:
    void busyWaitLoop(int64_t startLoop, int64_t durationLoop) {
        // beat starts with added PRINTF_WAIT_TIME shift to avoid interrupting thread on PRINTF
        constexpr auto BASE_PRINTF_BEAT = INIT_WAIT_TIME + PRINTF_WAIT_TIME;

        auto nextTick = TimeModel::computeNextBeat(BASE_PRINTF_BEAT, PRINT_INTERVAL, NOW());
        while(true) {
            // only print if beat is current (we don't print old PRINTFs "missed" in the past)
            if((nextTick <= NOW()) && (NOW() <= nextTick + WAIT_JITTER)) {
                xprintf("-");
                FFLUSH();
            }
            // PRINTF_WAIT_TIME shift however not included in break condition to avoid last wait
            if(NOW() >= startLoop + durationLoop) {
                return;
            }
            nextTick = TimeModel::computeNextBeat(BASE_PRINTF_BEAT, PRINT_INTERVAL, NOW());
            busyWaitUntil(nextTick);
        }
    }
};


/******************/

/**
 * @name Order of instantiation matters
 * (e.g. PriorityCeiler must be scheduled before LowPriorityThread initially)
 * @{
 */
HighPriorityThread   highPriorityThread;
LowPriorityThread    lowPriorityThread;
PriorityCeilerThread priorityCeiler;
/** @} */

/******************/

void MAIN() {
    printfMask = 1;
    if(Application::findApplication(2000) == 0) {
        xprintf("\n\n**********************\n");
        xprintf("To be able to test it compile together with PreemptionTest\n");
        xprintf("**********************\n\n\n");
    } else {
        xprintf("PreemptionTest found\n");
    }
}
