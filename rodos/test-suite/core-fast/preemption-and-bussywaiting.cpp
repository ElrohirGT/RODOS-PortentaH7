#include "rodos.h"

uint32_t printfMask = 0;

class LowPriorityThread : public StaticThread<> {
  public:
    static constexpr auto PRINT_INTERVAL = 10 * MILLISECONDS;
    static constexpr auto WAIT_JITTER    = 2 * MILLISECONDS;

    LowPriorityThread() : StaticThread<>("LowPriority", 100) {}

    void run() {
        printfMask = 1;
        PRINTF("\nLow prio begins printing '.'\n");

        while(true) {
            PRINTF(".");
            FFLUSH();

            Thread::suspendCallerUntil(TimeModel::computeNextBeat(0, PRINT_INTERVAL, NOW()));

            auto nextTick = TimeModel::computeNextBeat(0, PRINT_INTERVAL, NOW());
            while(NOW() < nextTick) {}
        }
    }
} lowPrio;

//__________________________________________________________

class HiPriorityThread : public StaticThread<> {
  public:
    static constexpr auto PRINT_INTERVAL = 10 * MILLISECONDS;

    /*
     * Wait period before printing is needed to prevents threads from printing at same time
     * -> due to small shifts in the print interval random PRINTF reordering is avoided
     */
    static constexpr auto PRINTF_WAIT_TIME = 4 * MILLISECONDS;

    /*
     * Time period denominating the time jitter expected when working with time
     * -> time a suspend can be safely extended to allow simultaneous operations
     *    on another thread to finish (e.g. PRINTF triggering exactly on wakeup time)
     * -> time a PRINTF can be delayed before it is dropped (not executed anymore)
     */
    static constexpr auto WAIT_JITTER = 2 * MILLISECONDS;

    static constexpr auto INIT_WAIT_TIME = 100 * MILLISECONDS;

    static constexpr auto START_LOOP_1    = INIT_WAIT_TIME;
    static constexpr auto DURATION_LOOP_1 = 200 * MILLISECONDS;

    static constexpr auto START_LOOP_2    = START_LOOP_1 + DURATION_LOOP_1;
    static constexpr auto DURATION_LOOP_2 = 400 * MILLISECONDS;

    HiPriorityThread() : StaticThread<>("HiPriority", 200) {}

    void run() {
        printfMask = 1;

        PRINTF("\nHi prio waits and then prints '+'\n");
        // add WAIT_JITTER to suspending to allow last PRINTF in other thread to finish
        suspendCallerUntil(INIT_WAIT_TIME + WAIT_JITTER);

        PRINTF("\nHi prio in busy waiting\n");
        busyWaitLoop(START_LOOP_1, DURATION_LOOP_1);

        /*
         * HiPriorityThread in PRIORITY_FLOORING schedule (time diagram sketch)
         * -> HiPriorityThread print period is shifted a bit (by respective PRINTF_WAIT_TIME)
         *
         * LowPriorityThread: P - - - x x x x P - - - x x x x P - - - x x x x P - - -
         * HiPriorityThread:  - - P x x x P x x x P x x x P x x x P x x x P x x x P x
         * threads scheduled: L H H H L L L L L H H H L L L L L H H H L L L L L H H H
         * threads printing:  L   H           L   H           L   H           L   H
         *
         * -> 'P': thread prints
         * -> 'x': thread is busy waiting
         * -> '-': thread is suspending
         * -> 'L': LowPriorityThread is scheduled/prints
         * -> 'H': HiPriorityThread is scheduled/prints
         */
        PRINTF("\nHi prio in busy waiting in PRIORITY_FLOORING\n");
        {
            PRIORITY_FLOORING_IN_SCOPE();
            yield();

            busyWaitLoop(START_LOOP_2, DURATION_LOOP_2);
        }
        PRINTF("\nHi prio ends\n");
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
                PRINTF("+");
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
} hiPri{};
