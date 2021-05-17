#ifndef UTIL_H_
#define UTIL_H_

#ifdef NO_STDINT_H
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

extern int sinlut[];

#define SIN(x) sinlut[(x) & 0x3ff]
#define COS(x) sinlut[((x) + 256) & 0x3ff]

int mask_to_shift(unsigned int mask);

#endif	/* UTIL_H_ */
