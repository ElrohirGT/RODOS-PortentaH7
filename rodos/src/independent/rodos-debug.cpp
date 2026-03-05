/**
* @file rodos-debug.cpp
* @date 2008/06/17 10:48
* @author Lutz Dittrich & Sergio Montenegro,
*   adapted in 2010 by Henrik Theiling (AbsInt Angewandte Informatik GmbH)
*   SNPRINTF added in 2019 by Sebastian Rückerl (TUM
*
* @brief debug and printf functions
*
*/

#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"


#include "misc-rodos-funcs.h"
#include "rodos-debug.h"
#include "rodos-semaphore.h"
#include "timepoints.h"
#include "yprintf.h"

#include "topic.h" // For Printf-over-middleware

namespace RODOS {

bool printErrorReports = true; // the user can set it to false

Semaphore printfProtector;

void PRINTF(const char* fmt, ...) {
    if(printfMask == 0) return;
    Yprintf yprintf;
    va_start(yprintf.ap, fmt);
    if (!isSchedulerRunning()) {
        yprintf.vaprintf(fmt);
    } else {
        printfProtector.enter();
            yprintf.vaprintf(fmt);
        printfProtector.leave();
    }
    FFLUSH();
}

void PRINTF_CONDITIONAL(uint32_t id, const char* fmt, ...) {
    if(id != 0xffffffff && (id & printfMask) == 0) return;
    Yprintf yprintf;
    va_start(yprintf.ap, fmt);
    if (!isSchedulerRunning()) {
        yprintf.vaprintf(fmt);
    } else {
        printfProtector.enter();
            yprintf.vaprintf(fmt);
        printfProtector.leave();
    }
    FFLUSH();
}

//_________________________________________________________________-


struct PrintBufferDescriptor printDescriptor;

Topic<PrintBufferDescriptor> printTopic(TOPIC_FOR_PRINTF, "printMessages");

Semaphore mwPrintProtector;

void MW_PRINTF (const char* fmt, ...) {

    mwPrintProtector.enter();
    // don't touch this!
    Ysprintf ysprintf(printDescriptor.buffer);
    va_start(ysprintf.ap, fmt);

    ysprintf.vaprintf(fmt);

    uint32_t len = (uint32_t)strlen(printDescriptor.buffer) + 9; //actual length of transimitted data combined with metadata
    printDescriptor.stringLength  = (uint32_t)strlen(printDescriptor.buffer);
    printDescriptor.sendingThread = (uint32_t)((uint64_t)Thread::getCurrentThread() & 0xffffffff); // use pointer to thread as unique_id
    printTopic.publishMsgPart(printDescriptor, len);
    mwPrintProtector.leave();

}

//_________________________________________________________________-


void RODOS_ERROR(const char* text) {
  rodosErrorMsg = text;
  rodosErrorCounter++;
  errorLog.addRaw(text);
  if(printErrorReports) PRINTF(SCREEN_RED "\n!! Programm ERROR %s!!\n", text); // keep in red to make aware
}


//void printBin(unsigned char val) {
//	PRINTF(" ");
//	for(int i = 0; i < 8; i++) {
//		PRINTF("%c", (val & 0x80)? '1' : '0');
//		val = val << 1;
//	}
//}

void printCharsHex(const char* titel, const void* ptr, size_t len) {
	const unsigned char* vals = (const unsigned char*)ptr;
	PRINTF("%s", titel);
	for(size_t i = 0; i < len; i++) PRINTF("%02x ", vals[i]);
	PRINTF("\n");
}

void printCharsHex_Conditional(uint32_t id, const char* titel, const void* ptr, size_t len) {
        if((id & printfMask) == 0 ) return;
        printCharsHex(titel,  ptr, len); 
}

char* formatBinary(uint32_t val, size_t len, char* outputBuf) {
	val <<= 32u - len;
	for(size_t i = 0; i < len; i++) {
		outputBuf[i] = ( (val & 0x80000000)? '1' : '0');
		val <<= 1;
	}
	outputBuf[len] = 0;
	return outputBuf;
}

class SNYprintf : public Yprintf {
public:
    char* dest;
    size_t max_size;
    size_t cur_size;

    SNYprintf(char* _dest, size_t _size)
        : dest(_dest)
        , max_size(_size)
        , cur_size(0)      {}

    ~SNYprintf() {
        if (max_size != 0 ) { 
            *dest = '\0'; 
        }
    }

    void yputc(char c) {  
        if (cur_size + 1 < max_size) {
            *dest = c;
            dest++;
            cur_size++;
        }
    }
};

size_t vsnprintf(char*dest, size_t size, const char* fmt, va_list args) {
    SNYprintf yprintf(dest, size);
    va_copy(yprintf.ap, args);
    if (!isSchedulerRunning()) {
        yprintf.vaprintf(fmt);
    } else {
        printfProtector.enter();
            yprintf.vaprintf(fmt);
        printfProtector.leave();
    }
    return yprintf.cur_size;
}

size_t SNPRINTF(char * dest, size_t size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t written_size = vsnprintf(dest, size, fmt, args);
    va_end(args);
    return written_size;
}

void SPRINTF(char* dest, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(dest, SPRINTF_MAX_SIZE, fmt, args);
    va_end(args);
}



}


