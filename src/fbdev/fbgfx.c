#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "fbgfx.h"

static void cleanup(void);

static int fd = -1;
static void *vmem;
static int vmem_size;
static struct fb_fix_screeninfo finfo;
static struct fb_var_screeninfo vinfo;
static struct fb_var_screeninfo saved_vinfo;
static int saved_vinfo_valid;
static struct fbgfx_vmode curvm;

static int init(void)
{
	if(fd >= 0) return 0;

	if((fd = open("/dev/fb0", O_RDWR)) == -1) {
		fprintf(stderr, "failed to open framebuffer device\n");
		return -1;
	}

	ioctl(fd, FBIOGET_FSCREENINFO, &finfo);

	curvm.pitch = finfo.line_length;

	atexit(cleanup);
	return 0;
}

static void cleanup(void)
{
	if(vmem) {
		munmap(vmem, vmem_size);
	}
	if(fd != -1) {
		close(fd);
	}
}

void *fbgfx_set_video_mode(int x, int y, int depth)
{
	struct fb_var_screeninfo new_vinfo;

	if(init() == -1) {
		return 0;
	}

	ioctl(fd, FBIOGET_VSCREENINFO, &new_vinfo);
	new_vinfo.xres = x;
	new_vinfo.yres = y;
	new_vinfo.bits_per_pixel = depth;

	if(ioctl(fd, FBIOPUT_VSCREENINFO, &new_vinfo) == -1) {
		fprintf(stderr, "failed to set video mode %dx%d %dbpp: %s\n", x, y, depth, strerror(errno));
		return 0;
	}

	if(vmem) {
		munmap(vmem, vmem_size);
		vmem = 0;
		vmem_size = 0;
	}

	return fbgfx_get_video_mode(0, 0, 0);
}

void *fbgfx_get_video_mode(int *xptr, int *yptr, int *depthptr)
{
	if(init() == -1) {
		return 0;
	}

	ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
	/*vmem_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;*/
	vmem_size = finfo.smem_len;

	curvm.width = vinfo.xres;
	curvm.height = vinfo.yres;
	curvm.bpp = vinfo.bits_per_pixel;
	curvm.fbsize = vmem_size;
	curvm.rshift = vinfo.red.offset;
	curvm.gshift = vinfo.green.offset;
	curvm.bshift = vinfo.blue.offset;
	curvm.rmask = (0xffffffff >> (32 - vinfo.red.length)) << curvm.rshift;
	curvm.gmask = (0xffffffff >> (32 - vinfo.green.length)) << curvm.gshift;
	curvm.bmask = (0xffffffff >> (32 - vinfo.blue.length)) << curvm.bshift;

	if(!vmem) {
		if((vmem = mmap(0, vmem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == (void*)-1) {
			fprintf(stderr, "failed to map video memory\n");
			vmem = 0;
			return 0;
		}
	}
	if(xptr) *xptr = vinfo.xres;
	if(yptr) *yptr = vinfo.yres;
	if(depthptr) *depthptr = vinfo.bits_per_pixel;
	return vmem;
}

void fbgfx_save_video_mode(void)
{
	if(init() == -1) {
		return;
	}

	if(ioctl(fd, FBIOGET_VSCREENINFO, &saved_vinfo) == -1) {
		return;
	}
	saved_vinfo_valid = 1;
}

void fbgfx_restore_video_mode(void)
{
	if(init() == -1 || !saved_vinfo_valid) {
		return;
	}
	ioctl(fd, FBIOPUT_VSCREENINFO, &saved_vinfo);
}

struct fbgfx_vmode *fbgfx_video_mode_info(void)
{
	return &curvm;
}
