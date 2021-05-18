#include "util.h"

uint32_t perf_start_count, perf_interval_count;

int mask_to_shift(unsigned int mask)
{
	int s = 0;
	if(mask) {
		while(!(mask & 1)) {
			mask >>= 1;
			s++;
		}
	}
	return s;
}
