#include "rodos.h"
#include "gateway.h"



/******************************/

static UDPInOut udp(-50000);
static LinkinterfaceUDP linkinterfaceUDP(&udp);
static Gateway gateway1(&linkinterfaceUDP, true);

/******************************/

class Printfer : public StaticThread<> {
	int32_t myId;
public:
  Printfer(int32_t id) : StaticThread("Printfer just to call printf") { myId = id;}
  void run() {
    int32_t cnt = 10000;
    int64_t period = (1000 + myId * 500) * MILLISECONDS;
    TIME_LOOP(0, period) {
      PRINTF("Thread %d : counter = %d\n", myId,cnt);
      MW_PRINTF("Thread %d : counter = %d\n", myId,cnt);
      cnt++;
    }
  }
} printfer[2] = {1,2};

