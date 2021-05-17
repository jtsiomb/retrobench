#ifndef UTIL_H_
#define UTIL_H_

extern int sinlut[];

#define SIN(x) sinlut[(x) & 0xff]
#define COS(x) sinlut[((x) + 64) & 0xff]

int mask_to_shift(unsigned int mask);

#endif	/* UTIL_H_ */
