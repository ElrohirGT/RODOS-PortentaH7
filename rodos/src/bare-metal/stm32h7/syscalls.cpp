/*
 * syscalls template
 * Sergio Montenegro
 */


#include "rodos.h"

namespace RODOS {

void sp_partition_yield() { }
void FFLUSH()             { }
void enterSleepMode()     { }
void hwInitTime()         { }
//int getCurrentIRQ(){ return 0; }

int64_t hwGetNanoseconds() {
	uint32_t count = 0;
	int64_t returnTime = 0;

	// -> current time = nanoTime + 1 000 000 000 * countRegister/tim2Clock

	// Disable Interrupt is no solution here (wrong values caused by missed interrupt)

	// Read nanoTime twice, to make sure it has not changed while reading counter value
	do {
		returnTime = nanoTime;
		count = TIM_GetCounter(TIMx);
	} while(returnTime != nanoTime);


	/** low precision
	 * - nanos = 166ns * count (for tim2Clock = 6MHz)
	 * - nanos = 11ns * count (for tim2Clock = 84MHz)
	 * ! startup time might be wrong due to integer overflow !
	 */
	//int nanos = 1000000000/timerClock * count;

	/** high precision
	 * - nanos = 166,666666ns * count (for tim2Clock = 6MHz)
	 * - nanos = 11,904761s * count (for tim2Clock = 84MHz)
	 * - takes 4 times longer than low precision
	 */
	long long nanos = 0;
	nanos = 1000000000/(timerClock/1000000);
	nanos *= count;
	nanos /= 1000000;

	return returnTime + nanos;
}

void hwInit() {
    xprintf("\n\n This port is empty! Will do nothing, maybe only crash\n\n");
}

}
