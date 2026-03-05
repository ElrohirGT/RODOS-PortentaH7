#include "rodos.h"
#include "gateway.h"

#include "./topics.h"

Topic<Position> posTopic(topicIdPosition,  "posTopic"); // to receive (subscrived)
Topic<MyTime>   timeTopic(topicIdMyTime, "timeTopic"); // to send (pulish)

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
    myTime.msgIndex = 1000;
    TIME_LOOP(0, 1600 * MILLISECONDS) {
      myTime.msgIndex++;
      myTime.timeNow = NOW();
      PRINTF("_________________ sending MyTime: cnt, time %3d,%3.6f \n",  (int)myTime.msgIndex, (double)(myTime.timeNow)/SECONDS);
      timeTopic.publish(myTime);
    }
  }
} myPublisher;

struct PosReceiver : public Subscriber {
  PosReceiver() : Subscriber(posTopic, "pos Pinter") {}
  uint32_t put(const uint32_t topicId, const size_t len, void *msg, const NetMsgInfo &) override {
    Position* pos = (Position*)msg;
    PRINTF("Got topic=%d len =%d  : ", (int)topicId, (int)len);
    PRINTF("Name: %s, cnt %d, coordinates (%f,%f,%f)\n", pos->name, (int)(pos->cnt), pos->x, pos->y, pos->z);
    return true;
  }
} posReceiver;
