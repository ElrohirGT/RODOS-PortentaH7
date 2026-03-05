/**
 * @file event.cc
 * @date 2008/05/06 10:12
 * @author Sergio Montenegro, Lutz Dittrich
 *
 *
 * @brief %Event management
 */
#include "timeevent.h"

#include <limits>
#include <stdint.h>

#include "rodos-atomic.h"
#include "listelement.h"
#include "rodos-debug.h"
#include "misc-rodos-funcs.h"

namespace RODOS {

/* constructor */
TimeEvent::TimeEvent(const char* name)
  : ListElement(TimeEvent::timeEventList, name),
    eventAt{ END_OF_TIME },
    eventPeriod{ 0 } {}


/* destructor */
TimeEvent::~TimeEvent() {
    if(isShuttingDown) return;
    RODOS_ERROR("Time EventHandler deleted");
}

/* Sets the time when the handler should be called
 * @param absolute time of next event
 */
void TimeEvent::activateAt(const int64_t time) {
    eventAt.store(time);
    eventPeriod.store(0);
}

/* defines the time relative to now, when the handler should be called: DEPRECATED */
// void TimeEvent::activateIn(const int64_t time) { activateAt(NOW() + time); }

/* like activateAt but with periodic reactivation */
void TimeEvent::activatePeriodic(const int64_t startAt, const int64_t period) {
    eventPeriod.store(period);
    eventAt.store(startAt);
}


/** TBA   Invoke event handler. Events are simply invoked by comparing event time and system time.
 * calls all time event handlers which eventAt < now (past)
 * and updates eventAt.
 * Returns the number of handle() called
 */
int32_t TimeEvent::propagate(const int64_t timeNow) {
    int32_t cnt = 0;
    ITERATE_LIST(TimeEvent, TimeEvent::timeEventList) {
        int64_t iterEventAt = iter->eventAt.load();
        if(iterEventAt < timeNow) {
            int64_t iterEventPeriod = iter->eventPeriod.load();
            if(iterEventPeriod == 0) { // not again until user sets it again
                iter->eventAt.store(END_OF_TIME);
            } else {
                iter->eventAt.store(iterEventAt + iterEventPeriod);
                if(iterEventAt < timeNow) { // Still in the past?
                    int64_t nextBeat =
                        TimeModel::computeNextBeat(iterEventAt, iterEventPeriod, timeNow);
                    iter->eventAt.store(nextBeat);
                }
            }
            iter->handle();
            cnt++;
        }
    }
    return cnt;
}

int64_t TimeEvent::getNextTriggerTime() {
    int64_t nextTriggerTime = std::numeric_limits<int64_t>::max();
    ITERATE_LIST(TimeEvent, TimeEvent::timeEventList) {
        auto currentEventTime = iter->eventAt.load();
        if(currentEventTime < nextTriggerTime) {
            nextTriggerTime = currentEventTime;
        }
    }
    return nextTriggerTime;
}

/* call init for each element in list */
int32_t TimeEvent::initAllElements() {
    int32_t cnt = 0;
    ITERATE_LIST(TimeEvent, TimeEvent::timeEventList) {
        iter->init();
        cnt++;
    }
    return cnt;
}

} // namespace RODOS
