#include "rodos.h"
#include "topics.h"

static CommBuffer<Pos> posBuf;
static CommBuffer<double> speedBuf;
static Subscriber nameNotImportant1(position, posBuf, "posReceiverBuf");
static Subscriber nameNotImportant2(speed, speedBuf, "speedReceiverBuf"); // fill buffer with published data

class Display : public StaticThread<> {
    void run() {
        TIME_LOOP(1 * SECONDS, 1 * SECONDS) {
            Pos p;
            double v;

            // get data from buffers
            posBuf.get(p);
            speedBuf.get(v);

            //print data
            PRINTF("Position (%3.2f;%3.2f;%3.2f) speed %3.2f\n", p.x, p.y, p.z, v);
        }
    }
} display;
