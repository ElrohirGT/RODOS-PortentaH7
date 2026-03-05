#include "rodos.h"
#include "random.h"


#include "bytestreamringbuffer.h"

/** For more variations see test-suite/core-fast/bytestreamringbuffer-test.cpp */

ByteStreamRingBuffer<256> rb; // 256 bytes but my buffer is int32_t, then maximal 64 entries

int32_t writeBuff[100];
int32_t readBuff[100];
int32_t longWordContent = 1000; // each long-word written shall have an unique content form 1000 to MAXINT

void MAIN() {

    RODOS_ASSERT(rb.isEmpty()); // Shall be empty


    PRINTF(" Store several (4) objects in buffer, and read all of them but do not overwrite old objects\n");

    for(uint32_t tryCnt = 1; tryCnt < 5; tryCnt++) { // going several times arround the buffer
        int32_t startValue[5], objectlen[5];
        objectlen[4] = 0; // we write only 4 the 5th shall be emty

        /** Write 4 messages **/
        for(int i = 0; i < 4; i++) {
            startValue[i] = longWordContent;
            int len       = (int)(uint32Rand() % (tryCnt * 10)) + 1; // no empty messages and message are getting bigger
            objectlen[i]  = len * 4;
            PRINTF(" Writing %d bytes counting from %d\n", objectlen[i], startValue[i]);
            for(int index = 0; index < len; index++) writeBuff[index] = longWordContent++;
            rb.write(writeBuff, len * 4);
        }

        /** try to read 5 objects, the last shall be emptys **/
        for(int i = 0; i < 5; i++) {
            int len = rb.read(readBuff, 400);
            PRINTF("Read  %d bytes, first objects: %d, %d, %d\n", len, readBuff[0], readBuff[1], readBuff[2]);
        }
        RODOS_ASSERT(rb.isEmpty());
    }

    hwResetAndReboot();
}
