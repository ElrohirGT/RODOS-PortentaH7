#include "rodos.h"
#include "gateway.h"

#include "./topics.h"

Topic<Position> posTopic(topicIdLinux2Rodos,  "posTopic"); // to receive (subscrived)
Topic<MyTime>   timeTopic(topicIdRodos2Linux, "timeTopic"); // to send (pulish)

/******************************/

static UDPInOut udp(udpPortNr);
static LinkinterfaceUDP linkinterfaceUDP(&udp);
static Gateway gateway1(&linkinterfaceUDP, true);

/******************************/

class MyPublisher : public StaticThread<> {
  MyTime myTime;
public:
  MyPublisher() : StaticThread("sender from RODOS") {}
  void run() {
    int32_t cnt = 10000;
    TIME_LOOP(0, 1600 * MILLISECONDS) {
      MW_PRINTF("sending %d\n", (int)cnt++);
      MW_PRINTF("sizeof(myTime) = %d\n", sizeof(myTime));
      myTime.msgIndex = cnt;
      myTime.timeNow  = NOW();
      timeTopic.publish(myTime);
    }
  }
} myPublisher;

struct PosReceiver : public Subscriber {
  PosReceiver() : Subscriber(posTopic, "pos Pinter") {}
  uint32_t put(const uint32_t topicId, const size_t len, void *msg, const NetMsgInfo &) override {
    Position* pos = (Position*)msg;
    PRINTF("Got topic=%d len =%d, sent to MW_PRINF  ", (int)topicId, (int)len);
    MW_PRINTF("topic=%d len =%d : ", (int)topicId, (int)len);
    MW_PRINTF(" Name: %s, cnt %d, coordinates (%f,%f,%f)\n", pos->name, (int)(pos->cnt), pos->x, pos->y, pos->z);
    return true;
  }
} posReceiver;
