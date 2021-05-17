#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

int init(void)
{
	printf("initialized graphics %dx%d %dbpp\n", fb_width, fb_height, fb_bpp);
	printf("  rgb mask: %x %x %x\n", fb_rmask, fb_gmask, fb_bmask);
	printf("  rgb shift: %d %d %d\n", fb_rshift, fb_gshift, fb_bshift);
	return 0;
}

void cleanup(void)
{
}

#define XORRGB(x, y, zoom, r, g, b) \
	do { \
		int xor = (((x) - fb_width/2) * (zoom) >> 10) ^ (((y) - fb_height/2) * (zoom) >> 10); \
		(r) = xor >> 2; \
		(g) = xor >> 1; \
		(b) = xor; \
	} while(0)

void redraw(void)
{
	int i, j, r, g, b, xoffs, yoffs, zoom;
	unsigned char *fbptr;
	uint16_t *fbptr16;
	uint32_t *fbptr32;

	xoffs = COS(time_msec >> 5) * fb_width >> 7;
	yoffs = SIN(time_msec >> 4) * fb_height >> 8;
	zoom = ((SIN(time_msec >> 4) + 256) << 1) + 512;

	switch(fb_bpp) {
	case 15:
	case 16:
		fbptr16 = framebuf;
		for(i=0; i<fb_height; i++) {
			for(j=0; j<fb_width; j++) {
				XORRGB(j + xoffs, i + yoffs, zoom, r, g, b);
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
				XORRGB(j + xoffs, i + yoffs, zoom, r, g, b);
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
				XORRGB(j + xoffs, i + yoffs, zoom, r, g, b);
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
