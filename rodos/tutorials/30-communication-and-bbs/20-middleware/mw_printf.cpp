#include <rodos.h>

class MwPrintfSender : public StaticThread<> {
public:
    MwPrintfSender() : StaticThread<>("MwPrintfSender") {}
    void run() {
        int32_t cnt = 1000;
        TIME_LOOP(1*SECONDS, 1*SECONDS) {
	    cnt++;
            PRINTF   ("local printf : %d\n", cnt);
            MW_PRINTF("remote printf: %d\n", cnt);
	    if(cnt > 1020) hwResetAndReboot();
        }
    }
} mwPrintfSender ;

class MwPrintfPrinter : public SubscriberReceiver<PrintBufferDescriptor> {
public:
    MwPrintfPrinter() : SubscriberReceiver<PrintBufferDescriptor>(printTopic, "MwPrintfPrinter") {}

    void put( PrintBufferDescriptor  &printbuf) override {
        PRINTF("from Thread %d,  %d bytes: %s\n", printbuf.sendingThread, printbuf.stringLength, printbuf.buffer);
    }
} mwPrintfPrinter;



