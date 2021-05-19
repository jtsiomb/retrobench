#ifndef FBGFX_H_
#define FBGFX_H_

struct fbgfx_vmode {
	int width, height, bpp;
	int pitch;
	unsigned int rmask, gmask, bmask;
	int rshift, gshift, bshift;
	int fbsize;
};

void *fbgfx_set_video_mode(int x, int y, int depth);
void *fbgfx_get_video_mode(int *xptr, int *yptr, int *depthptr);

void fbgfx_save_video_mode(void);
void fbgfx_restore_video_mode(void);

struct fbgfx_vmode *fbgfx_video_mode_info(void);


#endif	/* FBGFX_H_ */
