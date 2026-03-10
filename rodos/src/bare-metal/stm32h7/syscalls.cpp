/*
 * syscalls.cpp
 *
 *  Created on: 13.03.2012
 *      Author: Michael Ruffer
 */

#include <cstdio>
#include <rodos.h>
#include <stm32h7xx.h>
#include <h7_modules.h>
#include <platform-parameter.h>

namespace RODOS {

extern "C" {
int    putchar(int ic);
size_t _write(int file, const char* ptr, size_t len);
size_t _write([[gnu::unused]] int file, const char* ptr, size_t len) {
    size_t todo;

    for(todo = 0; todo < len; todo++) {

        putchar(*ptr);
        ptr++;
    }
    return len;
}

/*
 * reentrant syscalls
 */
size_t _write_r(void* reent, int fd, const void* buf, size_t cnt);
size_t _write_r([[gnu::unused]] void* reent, int fd, const void* buf, size_t cnt) { return _write(fd, (const char*)buf, cnt); }

// puts chars
int putchar(int ic) {
    const char c = (char)(ic & 0xff);

    if(c == '\n') { putchar('\r'); }

    // uart_stdout.putcharNoWait(c);

    const uint8_t value = (uint8_t)c;
    if(HAL_OK != HAL_UART_Transmit(UART_HANDLE, &value, 1, UART_TIMEOUT)) { // Thread Mode
        return EOF;
    }
    return c;
}

// puts strings
int puts(const char* str);
int puts(const char* str) {
    const char* c;
    c = str;
    while(*c) { putchar(*c++); }
    return 0;
}

void abort(void);
void abort(void) { while(1); }
}
} // namespace RODOS
