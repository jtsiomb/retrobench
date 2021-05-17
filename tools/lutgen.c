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
	for(i=0; i<1024; i++) {
		float x = sin((float)i / 512.0f * M_PI);
		printf("\t.long %d\n", (int)(x * 65536.0f));
	}
	return 0;
}
