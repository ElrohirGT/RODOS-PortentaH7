#include <rodos.h>

uint32_t printfMask = 0;

class MwPrintfSender : public StaticThread<> {
public:
    MwPrintfSender() : StaticThread<>("MwPrintfSender") {}
    void run() {
	printfMask = 1;
        for(int32_t cnt = 0; cnt < 110; cnt++) {
            PRINTF   ("local printf : %d\n", cnt);
            MW_PRINTF("remote printf: %d\n", cnt);
        }
	hwResetAndReboot();
    }
} mwPrintfSender ;

class MwPrintfPrinter : public SubscriberReceiver<PrintBufferDescriptor> {
public:
    MwPrintfPrinter() : SubscriberReceiver<PrintBufferDescriptor>(printTopic, "MwPrintfPrinter") {}

    void put( PrintBufferDescriptor  &printbuf) override {
        PRINTF("%d bytes: %s",  printbuf.stringLength, printbuf.buffer);
    }
    
} mwPrintfPrinter;



