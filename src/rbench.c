#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rbench.h"
#include "treestor.h"
#include "util.h"

#define DEF_WIDTH	640
#define DEF_HEIGHT	480
#define DEF_BPP		24

struct options opt = {
	DEF_WIDTH, DEF_HEIGHT, DEF_BPP
};

int fb_width, fb_height, fb_bpp, fb_pitch;
int fb_rshift, fb_gshift, fb_bshift;
unsigned int fb_rmask, fb_gmask, fb_bmask;
void *framebuf;
unsigned int time_msec;

static const char *cpufeat[] = {
	"fpu", "vme", "dbgext", "pse", "tsc", "msr", "pae", "mce", "cx8", "apic", 0,
	"sep", "mtrr", "pge", "mca", "cmov", "pat", "pse36", "psn", "clf", 0,
	"dtes", "acpi", "mmx", "fxsr", "sse", "sse2", "ss", "htt", "tm1", "ia64", "pbe"
};

static const char *cpufeat2[] = {
	"sse3", "pclmul", "dtes64", "monitor", "dscpl", "vmx", "smx", "est", "tm2",
	"ssse3", "cid", 0, "fma", "cx16", "etprd", "pdcm", 0, "pcide", "dca", "sse4.1",
	"sse4.2", "x2apic", "movbe", "popcnt", 0, "aes", "xsave", "osxsave", "avx"
};

int init(void)
{
	int i;
	struct cpuid_info cpu;

	if(read_cpuid(&cpu) != -1) {
		printf("CPUID information:\n");
		printf("  cpuid blocks: %d\n", (int)cpu.maxidx);
		printf("  CPU vendor: ");
		for(i=0; i<12; i++) {
			putchar(cpu.vendor[i]);
		}
		putchar('\n');
		printf("  stepping: %u, model: %u, family: %u\n", CPUID_STEPPING(cpu.id),
				CPUID_MODEL(cpu.id), CPUID_FAMILY(cpu.id));
		printf("  features:");
		for(i=0; i<sizeof cpufeat / sizeof *cpufeat; i++) {
			if(cpufeat[i] && (cpu.feat & (1 << i))) {
				printf(" %s", cpufeat[i]);
			}
		}
		for(i=0; i<sizeof cpufeat2 / sizeof *cpufeat2; i++) {
			if(cpufeat2[i] && (cpu.feat2 & (1 << i))) {
				printf(" %s", cpufeat2[i]);
			}
		}
		putchar('\n');
	}

	printf("initialized graphics %dx%d %dbpp\n", fb_width, fb_height, fb_bpp);
	printf("  rgb mask: %x %x %x\n", fb_rmask, fb_gmask, fb_bmask);
	printf("  rgb shift: %d %d %d\n", fb_rshift, fb_gshift, fb_bshift);
	return 0;
}

void cleanup(void)
{
}

#ifdef NOZOOM
#define XORRGB(x, y, dx, dy, zoom, r, g, b) \
	do { \
		int xor = (((x) - fb_width/2) + (dx)) ^ (((y) - fb_height/2) + (dy)); \
		(r) = xor >> 2; \
		(g) = xor >> 1; \
		(b) = xor; \
	} while(0)
#else
#define XORRGB(x, y, dx, dy, zoom, r, g, b) \
	do { \
		int xor = ((((x) - fb_width/2) * (zoom) >> 16) + (dx)) ^ ((((y) - fb_height/2) * (zoom) >> 16) + (dy)); \
		(r) = xor >> 2; \
		(g) = xor >> 1; \
		(b) = xor; \
	} while(0)
#endif

void redraw(void)
{
	int i, j, r, g, b, xoffs, yoffs;
#ifndef NOZOOM
	int zoom;
#endif
	unsigned char *fbptr;
	uint16_t *fbptr16;
	uint32_t *fbptr32;

	xoffs = COS(time_msec >> 2) * fb_width >> 14;
	yoffs = SIN(time_msec >> 1) * fb_height >> 15;
#ifndef NOZOOM
	zoom = (SIN(time_msec >> 3) << 1) + 0x18000;
#endif

	switch(fb_bpp) {
	case 15:
	case 16:
		fbptr16 = framebuf;
		for(i=0; i<fb_height; i++) {
			for(j=0; j<fb_width; j++) {
				XORRGB(j, i, xoffs, yoffs, zoom, r, g, b);
				*fbptr16++ = (((r >> 3) << fb_rshift) & fb_rmask) |
					(((g >> 2) << fb_gshift) & fb_gmask) |
					(((b >> 3) << fb_bshift) & fb_bmask);
			}
			fbptr16 += (fb_pitch >> 1) - fb_width;
		}
		break;

	case 24:
		fbptr = framebuf;
		for(i=0; i<fb_height; i++) {
			for(j=0; j<fb_width; j++) {
				XORRGB(j, i, xoffs, yoffs, zoom, r, g, b);
				*fbptr++ = r;
				*fbptr++ = g;
				*fbptr++ = b;
			}
			fbptr += fb_pitch - fb_width * 3;
		}
		break;

	case 32:
		fbptr32 = framebuf;
		for(i=0; i<fb_height; i++) {
			for(j=0; j<fb_width; j++) {
				XORRGB(j, i, xoffs, yoffs, zoom, r, g, b);
				*fbptr32++ = (((r) << fb_rshift) & fb_rmask) |
					(((g) << fb_gshift) & fb_gmask) |
					(((b) << fb_bshift) & fb_bmask);
			}
			fbptr32 += (fb_pitch >> 2) - fb_width;
		}
		break;
	}
}

void key_event(int key, int press)
{
}

int read_config(const char *fname)
{
	FILE *fp;
	struct ts_node *ts;

	if(!(fp = fopen(fname, "rb"))) {
		return -1;
	}
	fclose(fp);

	if(!(ts = ts_load(fname))) {
		return -1;
	}

	opt.width = ts_lookup_int(ts, "rbench.width", DEF_WIDTH);
	opt.height = ts_lookup_int(ts, "rbench.height", DEF_HEIGHT);
	opt.bpp = ts_lookup_int(ts, "rbench.bpp", DEF_BPP);

	ts_free_tree(ts);
	return 0;
}
