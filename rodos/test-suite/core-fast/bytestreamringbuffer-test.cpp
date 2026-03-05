#ifdef ON_LINUX
#    include "stdio.h"
#    include <stdlib.h>
#    include <cassert>

/** Map rodos calls to linux calls **/
#    define RODOS_ASSERT assert
#    define PRINTF printf
#    define uint32Rand rand
#    define hwResetAndReboot()

#else
#    include "rodos.h"
#    include "random.h"

#endif

#include "bytestreamringbuffer.h"

uint32_t printfMask = 0;

ByteStreamRingBuffer<256> rb; // 256 bytes but my buffer is int32_t, then maximal 64 entries

int32_t writeBuff[100];
int32_t readBuff[100];
int32_t longWordContent = 1000; // each long-word written shall have an unique content form 1000 to MAXINT

#ifdef ON_LINUX
int main() {
#else
void MAIN() {
#endif

    printfMask = 1;
    RODOS_ASSERT(rb.isEmpty()); // Shall be empty

    PRINTF(" only one object in the buffer from 0 to max len\n");

    for(int tryCnt = 0; tryCnt < 50; tryCnt++) { // going several times arround the buffer
        for(int tryLen = 1; tryLen < 60; tryLen++) {

            /** normal write and read **/
            for(int index = 0; index < tryLen; index++) writeBuff[index] = longWordContent++;
            RODOS_ASSERT(rb.write(writeBuff, tryLen * 4));
            RODOS_ASSERT(!rb.isEmpty());
            RODOS_ASSERT(rb.read(readBuff, 400) == tryLen * 4);
            for(int index = 0; index < tryLen; index++) RODOS_ASSERT(readBuff[index] == writeBuff[index]);
            RODOS_ASSERT(rb.isEmpty());

            /** read to buffer shorter than object len **/
            RODOS_ASSERT(rb.write(writeBuff, tryLen * 4));
            RODOS_ASSERT(!rb.isEmpty());
            int readLen = rb.read(readBuff, 16);
            RODOS_ASSERT((readLen == -1 && tryLen * 4 > 16) || (readLen == tryLen * 4));
            RODOS_ASSERT(rb.isEmpty());
        }
    }

    PRINTF(" Store several (4) objects in buffer, and read all of them but do not overwrite old objects\n");

    for(int tryCnt = 0; tryCnt < 50; tryCnt++) { // going several times arround the buffer
        int32_t startValue[5], objectlen[5];
        objectlen[4] = 0;

        /** Write 4 messages **/
        for(int i = 0; i < 4; i++) {
            startValue[i] = longWordContent;
            int32_t len   = uint32Rand() % 16 + 1; // no empty messages
            objectlen[i]  = len * 4;
            for(int index = 0; index < len; index++) writeBuff[index] = longWordContent++;
            RODOS_ASSERT(rb.write(writeBuff, len * 4));
        }

        /** try to read 5 objects, the last shall be emptys **/
        for(int i = 0; i < 5; i++) {
            RODOS_ASSERT(rb.read(readBuff, 400) == objectlen[i]);
            for(int index = 0; index < objectlen[i] / 4; index++) RODOS_ASSERT(readBuff[index] == startValue[i] + index);
        }
        RODOS_ASSERT(rb.isEmpty());
    }

    PRINTF(" Store many (20) objects in buffer to overwrite old objects, read until empty\n");

    for(int tryCnt = 0; tryCnt < 20; tryCnt++) { // going several times arround the buffer
        int32_t startValue[20], objectlen[20];

        /** Write 20 messages **/
        for(int i = 0; i < 20; i++) {
            int32_t len   = uint32Rand() % 16 + 1; // no empty messages
            startValue[i] = longWordContent;
            objectlen[i]  = len * 4;
            for(int index = 0; index < len; index++) writeBuff[index] = longWordContent++;
            RODOS_ASSERT(rb.write(writeBuff, len * 4));
        }

        /** Read one object and find which of the 20 is it **/
        int32_t len = rb.read(readBuff, 400);
        int32_t firstObj;
        for(firstObj = 0; firstObj < 20; firstObj++) {
            if(readBuff[0] == startValue[firstObj]) break;
        }
        // PRINTF("   first object found nr. (0..19) %d, len = %d\n", firstObj, len);
        RODOS_ASSERT(firstObj < 20);
        RODOS_ASSERT(len < 68); // accorint to 4*(uint32Rand() % 16 + 1)
        int32_t expectedContent = readBuff[0];
        for(int index = 0; index < len / 4; index++) RODOS_ASSERT(readBuff[index] = expectedContent++);

        /** read until empty **/
        while((len = rb.read(readBuff, 400) != 0)) {
            // PRINTF("    reading next, len = %d\n", len);
            for(int index = 0; index < len / 4; index++) RODOS_ASSERT(readBuff[index] == expectedContent++);
        }
    }

    PRINTF(" Write 2, read one, keep the buffer full \n");

    static const int32_t expectedMaxNumberOfObjects = 50;
    int32_t              startValue[expectedMaxNumberOfObjects], objectlen[expectedMaxNumberOfObjects]; // as ring buffers
    size_t               writeX = 0;
    for(int tryCnt = 0; tryCnt < 1000; tryCnt++) { // going several times arround the buffer

        for(int i = 0; i < 2; i++) {
            int32_t len        = uint32Rand() % 16 + 1; // no empty messages
            startValue[writeX] = longWordContent;
            objectlen[writeX]  = len * 4;
            writeX             = (writeX + 1) % expectedMaxNumberOfObjects;
            for(int index = 0; index < len; index++) writeBuff[index] = longWordContent++;
            RODOS_ASSERT(rb.write(writeBuff, len * 4));
        }

        /** Read one object and find which coult it be in the last N written objects **/
        int32_t len = rb.read(readBuff, 400);
        int32_t firstObj;
        for(firstObj = 0; firstObj < expectedMaxNumberOfObjects; firstObj++) {
            if(readBuff[0] == startValue[firstObj]) break;
        }
        // PRINTF("   first object found nr. (0..49) %d, len = %d\n", firstObj, len);
        RODOS_ASSERT(firstObj < expectedMaxNumberOfObjects);
        RODOS_ASSERT(len == objectlen[firstObj]);
        int32_t expectedContent = readBuff[0];
        for(int index = 0; index < len / 4; index++) RODOS_ASSERT(readBuff[index] = expectedContent++);
    }


    PRINTF(" Write and read 4 object with even and odd len \n");

    rb.reset();
    uint8_t* writeBytes = (uint8_t*)writeBuff;
    uint8_t* readBytes  = (uint8_t*)readBuff;
    // writeX = 0;

    for(int tryCnt = 0; tryCnt < 50; tryCnt++) { // going several times arround the buffer
        uint8_t byteContent = 0;
        int32_t startValue[5], objectlen[5];
        objectlen[4] = 0;

        for(int i = 0; i < 4; i++) {
            startValue[i] = byteContent;
            int len       = uint32Rand() % 15 + 1; // no empty messages
            objectlen[i]  = len;
            for(int index = 0; index < len; index++) writeBytes[index] = byteContent++;
            RODOS_ASSERT(rb.write(writeBytes, len));
        }

        /** try to read 5 objects, the last shall be emptys **/
        for(int i = 0; i < 5; i++) {
            RODOS_ASSERT(rb.read(readBytes, 400) == objectlen[i]);
            for(int index = 0; index < objectlen[i]; index++) RODOS_ASSERT(readBytes[index] == startValue[i] + index);
        }
        RODOS_ASSERT(rb.isEmpty());
    }


    PRINTF("\n\n If you are reading this line, this means you can read and I found no disconformities\n");
    hwResetAndReboot();
}
