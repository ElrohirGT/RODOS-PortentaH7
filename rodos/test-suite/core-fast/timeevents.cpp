/**
* @file timeevents.cc
* @date 2008/06/11 16:07
* @author Sergio Montenegro, Lutz Dittrich
*
*
* @brief test of time event management
*/

#include "rodos.h"

uint32_t printfMask = 0;    //Will be set to 1 in init of TimeEventTest3 to get the PRINTF then to 0 and the in the handle functions to 1

static Application module01("TestTimeEvents");

class TimeEventTest1 : public TimeEvent {
  protected:
    void handle() {
        printfMask = 1;
        xprintf("[TimeEventTest1::handle] SINGLE TimeEvent at %3.3fs\n", SECONDS_NOW());
    }

    void init() {
        activateAt(25 * MILLISECONDS);
    }
};

static TimeEventTest1 te01;

class TimeEventTest2 : public TimeEvent {
  public:
    void handle() {
        printfMask = 1;
        xprintf("[TimeEventTest2::handle] Periodic TimeEvent at %3.3fs\n", SECONDS_NOW());
    }

    void init() {
        activatePeriodic(40 * MILLISECONDS, 10 * MILLISECONDS);
    }
};

static TimeEventTest2 te02;

class TimeEventTest3 : public TimeEvent {
    int i = 0;
  protected:
    void handle() {
        printfMask = 1;
        i++;
        if (i > 10) {
            PRINTF("Terminating\n");
            FFLUSH();
            hwResetAndReboot();
        }
        xprintf("[TimeEventTest3::handle] TimeEvent at %3.3f, reactivation in 0.02s\n", SECONDS_NOW());
        activateAt(NOW() + 20 * MILLISECONDS);
    }

    void init() {
        printfMask = 1;
        printfMask = 0;
        activateAt(NOW() + 12 * MILLISECONDS);
    }
};

static TimeEventTest3 te03;
