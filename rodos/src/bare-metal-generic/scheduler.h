
/**
* @file scheduler.h
* @date 2008/04/24 16:23
* @author Lutz Dittrich
*
*
* @brief priority based %scheduler (header)
*
*/

#pragma once


#include "rodos-atomic.h"
#include "thread.h"


namespace RODOS {

extern "C" {
  /**
  * This is a wrapper function with C linkage.
  * It stores the context and calls the scheduler.
  */
  void schedulerWrapper(long* ctx) __attribute__((used));
}

/**
* @class Scheduler
* @brief A class for the scheduling algorithm.
*
* Priority based scheduler.
*/
class Scheduler {

friend class Thread;
private:
  static RODOS::AtomicRO<uint64_t> scheduleCounter; // shared with Thread::yield()

  static Thread* preSelectedNextToRun; // used only to optimize yield time
  static int64_t preSelectedEarliestSuspendedUntil; // used only to optimize yield time

public:
  /**
   * Call the scheduling algorithm.
   */
  static Thread* schedule();

  /**
   * Activate the idle thread.
   */
  static void idle();

  /**
   * Getter for scheduleCounter.
   */
  static uint64_t getScheduleCounter();

};


} // namespace

