#ifndef SWIT3_INT_H
#define SWIT3_INT_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>

#include "types.h"

#ifdef MODBUS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "web.h"
#endif

#ifdef WEB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "web.h"
#endif

#define WSP(c) (((c-1) >> 5) == 0)
#define PRS2(c) ((((c | 32)) - 32))
#define PRS(c) (((PRS2(c) & 31)) | ((PRS2(c) & ~(31)) >> 1))
#define H3(a,b,c) ((PRS(a) << 12) | (PRS(b) << 6) | (PRS(c)))
#define Hs(s) (H3(s[0], s[1], s[2]))
#define H2s(s) (H3(0, s[0], s[1]))
#define H1s(s) (H3(0, 0, s[0]))
#define H(c) (H3(0, 0, c))
#define H2(c,d) (H3(0, c, d))
// #define NUM(c) ((PRS2(c) >> 5) == 0)
#define NUM(c) (((c - 32) >> 5) == 0)

#define GET_TIME() ((float)clock() / (CLOCKS_PER_SEC))
#define SAVE_START_TIME() float bench_start_time = GET_TIME()
#define PRINT_TIME_FROM_START() (printf("in %.3f ms\n", ((GET_TIME() - bench_start_time) * 1000)))

// x = SM2^e
#define F16MANBITS 10
#define F16EXPBITS (15-(F16MANBITS))
#define F16MASK(n) ((1 << (n)) - 1)
#define F16MANMAX (F16MASK(F16MANBITS))
#define F16EXPMAX (F16MASK(F16EXPBITS))
#define F16PACK(sign, exp, frac) (((sign & 1) << (F16MANBITS + F16EXPBITS)) | ((exp & F16MASK(F16EXPBITS)) << (F16MANBITS)) | (frac & F16MASK(F16MANBITS)))
#define F16SIGN(f) ((f >> (F16MANBITS + F16EXPBITS)) & 1)
#define F16EXP(f) ((f >> (F16MANBITS)) & F16MASK(F16EXPBITS))
#define F16MAN(f) (f & F16MASK(F16MANBITS))

#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)

#define PRINTF_BINARY_PATTERN_INT4 "%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT4(i)    \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

int8_t interpret(struct Interpreter *ci);

#endif