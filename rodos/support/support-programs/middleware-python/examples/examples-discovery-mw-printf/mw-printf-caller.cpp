#include "rodos.h"
#include "gateway.h"


/******************************/
//uart for uart->bluetooth->linux->python->mwinterface
static HAL_UART uart(UART_IDX2); // USB-UART      Tx:= PD5 , Rx:= PD6
static LinkinterfaceUART linkif(&uart, 115200, 3, 10);
static Gateway gw(&linkif, true);

/******************************/


class Printfer : public StaticThread<> {
	int32_t myId;
public:
  Printfer(int32_t id) : StaticThread("Printfer just to call printf") { myId = id;}
  void run() {
    int32_t cnt = 10000;
    int64_t period = (300 + myId * 100) * MILLISECONDS;
    TIME_LOOP(0, period) {
      PRINTF("Serial Debug: Thread %d : counter = %d\n", myId,cnt); //uncomment only if uart debug is set to different UART_IDX3 in platform-parameter.h
      MW_PRINTF("Thread %d : counter = %d\n", myId,cnt);
      cnt++;
    }
  }
}printfer[2] = {1,2};
