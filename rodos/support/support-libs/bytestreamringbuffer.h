#pragma once
#include <stdint.h>

/**
 * @file bytestreamringbuffer.h
 * @date 2025/04/25
 * @author Sergio Montenegro, Simon Klüpfel
 *
 * Byte based ring buffer with variable lengths, suitable for RODOS, i.e. without dynamic memory allocation
 * The class uses a statically allocated memory area and explicitly stores the length of each entry
 * To support different objects (byte strings), e.g. byte streaming of objects.
 *
 * Warning 1: Not thread-safe
 *
 * Warning 2: Externally we use bytes as unit, internally we use long words (4 bytes) as unit
 *
 * Warning 3: All in/out buffers must be 16-bit (long word) aling, eg defie as array of int32_t
 *
 * Warning 4: Data transfers are allway multiple of 4 bytes but never longer than the MaxLen
 **/


template <size_t LEN> // LEN number of bytes. 2'er potences make it faster due to modulo operations !
class ByteStreamRingBuffer {

#define wordsInObject(bytes) ((bytes) + 3) / 4         /* long words uprounded */
  protected:
    static constexpr int WORDS = wordsInObject(LEN); // In this file, a Word is 32 Bits !
    int32_t              buffer[WORDS];

    int writeX = 0; //< index of where to write
    int readX  = 0; //< index of where to read

    inline void writeWord(int32_t val) {
        buffer[writeX] = val;
        writeX         = (writeX + 1) % WORDS;
        if(writeX == readX) readX = (readX + 1 + wordsInObject(buffer[readX])) % WORDS; // overwrite/jump one object Header (len) + object
    }

    inline int32_t readWord() {
        int32_t val = buffer[readX];
        readX       = (readX + 1) % WORDS;
        return val;
    }

  public:
    ByteStreamRingBuffer() { readX = writeX = 0; }

    inline bool isEmpty() { return readX == writeX; }
    inline void reset()   { readX = writeX = 0;     }

    /**
     * Write to the ring buffer a number of bytes.
     * Warning: Souce has to be 32 bit align
     */

    bool write(const void* buf, int len) {
        if(wordsInObject(len) >= WORDS) return false; // it does not fit
        if(len == 0) return true;                     // or shall I write empty messages?!? would produce read len 0, like whem empty

        writeWord((int32_t)len);
        const int32_t* source = (const int32_t*)buf;
        for(; len > 0; len -= 4) writeWord(*source++); // if len is not multiple of 4 it writes one more word for the rest few bytes
        return true;
    }

    /**
     * read and object from the ring buffer
     * it returns the number of bytes transfered to the output buf parameter (Multiple of 4)
     * Warning: destination has to be 32 bit align
     * Dielma; What shall I do if object > maxLen? 1. reject read? (external infinity loop), 2. incomplete read (this), 3. skip messae?
     */
    int read(void* buf, int maxLen) {
        if(isEmpty()) return 0;
        int len       = readWord(); // the first word in the object is the len
        int objectLen = len;
        if(objectLen > maxLen) {
            readX = (readX + wordsInObject(objectLen)) % WORDS; // Discard this message, to long to read!
            return -1;
        }

        int32_t* destination = (int32_t*)buf; // // if len is not multiple of 4 it transfer one more word for the rest few bytes
        for(; len > 0; len -= 4) *destination++ = readWord();
        return objectLen;
    }
};
