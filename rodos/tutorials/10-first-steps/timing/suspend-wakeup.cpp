#include "thread.h"
#include "timemodel.h"
#include "yprintf.h"

namespace RODOS {

class LowPrioThread : public StaticThread<> {
  public:
    LowPrioThread() : StaticThread<>("LowPrioThread", 100) {}

    void run() {
        busyWaitUntil(END_OF_TIME);
    }
} lowPrio{};

class HighPrioThread : public StaticThread<> {
  public:
    /// reactivation-time not aligned to SysTick (still should wakeup exactly)
    static constexpr auto REACTIVATION_TIME_1 = 1234 * MILLISECONDS;
    static constexpr auto REACTIVATION_TIME_2 = 2468 * MILLISECONDS;

    HighPrioThread() : StaticThread<>("HighPrioThread", 200) {}

    void run() {
        xprintf("[HighPrioThread] suspending until %3.9f\n", REACTIVATION_TIME_1 / (double)SECONDS);
        suspendCallerUntil(REACTIVATION_TIME_1);
        xprintf("[HighPrioThread] woke up again at %3.9f\n", SECONDS_NOW());

        xprintf("[HighPrioThread] busy-waiting until %3.9f\n", REACTIVATION_TIME_2 / (double)SECONDS);
        busyWaitUntil(REACTIVATION_TIME_2);
        xprintf("[HighPrioThread] woke up again at   %3.9f\n", SECONDS_NOW());
    }
} highPrio{};

} // namespace RODOS
