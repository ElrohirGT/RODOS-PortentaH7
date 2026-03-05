#include <cstdint>

#include "thread.h"
#include "timeevent.h"
#include "yprintf.h"

namespace RODOS {

constexpr auto INTERVAL = 250 * MILLISECONDS;

class PeriodicEvent : public TimeEvent {
  protected:
    void handle() {
        xprintf("> %3.9f\n", SECONDS_NOW());
    }

    void init() {
        this->activatePeriodic(0, INTERVAL);
    }
} periodicEvent{};

} // namespace RODOS
