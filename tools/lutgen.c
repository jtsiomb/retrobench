#include <stdio.h>
#include <math.h>

int main(void)
{
	int i;

	puts("\t.data");
	puts("\t.globl _sinlut");
	puts("\t.globl sinlut");
	puts("_sinlut:");
	puts("sinlut:");
	for(i=0; i<2048; i++) {
		float x = sin((float)i / 1024.0f * M_PI);
		printf("\t.word %d\n", (short)(x * 32767.0f));
	}
	return 0;
}
