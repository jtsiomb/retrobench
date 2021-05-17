#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "rbench.h"
#include "treestor.h"

#define DEF_WIDTH	640
#define DEF_HEIGHT	480
#define DEF_BPP		24

struct options opt = {
	DEF_WIDTH, DEF_HEIGHT, DEF_BPP
};

int fb_width, fb_height, fb_bpp, fb_pitch;
void *framebuf;
unsigned int time_msec;

int init(void)
{
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
	float t = (float)time_msec / 1000.0f;

	xoffs = cos(t * 0.5f) * DEF_WIDTH * 2;
	yoffs = sin(t) * DEF_HEIGHT;
	zoom = (sin(t * 0.75f) * 0.5f + 1.0f) * 1024.0f;

	switch(fb_bpp) {
	case 15:
		fbptr16 = framebuf;
		for(i=0; i<fb_height; i++) {
			for(j=0; j<fb_width; j++) {
				XORRGB(j + xoffs, i + yoffs, zoom, r, g, b);
				*fbptr16++ = ((r & 0x1f) << 10) | ((g & 0x1f) << 5) | (b & 0x1f);
			}
			fbptr16 += (fb_pitch >> 1) - fb_width;
		}
		break;

	case 16:
		fbptr16 = framebuf;
		for(i=0; i<fb_height; i++) {
			for(j=0; j<fb_width; j++) {
				XORRGB(j + xoffs, i + yoffs, zoom, r, g, b);
				*fbptr16++ = ((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f);
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
				*fbptr32++ = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
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
