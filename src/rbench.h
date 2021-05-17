#ifndef RETROBENCH_H_
#define RETROBENCH_H_

struct options {
	int width, height, bpp;
};

extern struct options opt;
extern int fb_width, fb_height, fb_bpp, fb_pitch;
extern int fb_rshift, fb_gshift, fb_bshift;
extern unsigned int fb_rmask, fb_gmask, fb_bmask;
extern void *framebuf;
extern unsigned int time_msec;

int init(void);
void cleanup(void);

void redraw(void);
void key_event(int key, int press);

int read_config(const char *fname);


#endif	/* RETROBENCH_H_ */
