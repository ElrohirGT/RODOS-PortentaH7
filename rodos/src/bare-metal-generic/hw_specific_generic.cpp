#include "hw_specific.h"

#include "default-platform-parameter.h"
#include "misc-rodos-funcs.h"
#include "timemodel.h"

namespace RODOS {

void Timer::updateTriggerToNextTimingEvent(
    int64_t nextSchedulingEventTime,
    int64_t nextTimeEventTriggerTime) {
#ifndef DISABLE_TIMEEVENTS
    int64_t reactivationTime = min(nextSchedulingEventTime, nextTimeEventTriggerTime);
#else
    int64_t reactivationTime = nextSchedulingEventTime;
#endif
    // don't set interval to less than MIN_SYS_TICK_SPACING
    // -> this is done to avoid flooding the system with SysTick interrupts
    // -> deadlines may be overrun by maximally:
    //    2 * MIN_SYS_TICK_SPACING (+ scheduling overhead + accumulated previous overrunning)
    int64_t timeNow = NOW();
    int64_t nextOfNextBeat =
        TimeModel::computeNextBeat(0, MIN_SYS_TICK_SPACING, timeNow + MIN_SYS_TICK_SPACING);
    reactivationTime = max(reactivationTime, nextOfNextBeat);
    int64_t intervalInNanoSecs = reactivationTime - timeNow;
    Timer::setInterval(intervalInNanoSecs / 1000l); // nanoseconds to microseconds
}

} // namespace RODOS
