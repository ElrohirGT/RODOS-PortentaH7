#include <cstdint>

#include "thread.h"
#include "yprintf.h"
#include "timemodel.h"

namespace RODOS {

constexpr auto INTERVAL = 250 * MILLISECONDS;

class PeriodicOutputThread : public StaticThread<> {
  protected:
    void run() {
        while(true) {
            busyWaitUntil(TimeModel::computeNextBeat(0, INTERVAL, NOW()));
            auto timeNow = SECONDS_NOW();
            xprintf("> %3.9f\n", timeNow);
        }
    }
} periodicOutputThread{};

} // namespace RODOS
