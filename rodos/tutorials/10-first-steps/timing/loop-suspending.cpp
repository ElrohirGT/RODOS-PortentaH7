#include <cstdint>

#include "thread.h"
#include "yprintf.h"

namespace RODOS {

constexpr auto INTERVAL = 250 * MILLISECONDS;

class PeriodicOutputThread : public StaticThread<> {
  protected:
    void run() {
        TIME_LOOP(0, INTERVAL) {
            xprintf("> %3.9f\n", SECONDS_NOW());
        }
    }
} periodicOutputThread{};

} // namespace RODOS
