#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "rbench.h"
#include "util.h"


static int init_fbdev(int xsz, int ysz, int bpp);
static void close_fbdev(void);
static int parse_args(int argc, char **argv);

static int fbfd = -1;
static int fbsz;
static void *fbmem;


int main(int argc, char **argv)
{
	int num_frames = 0;
	struct timeval tv, tv0;

	read_config("rbench.cfg");

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	if(init_fbdev(opt.width, opt.height, opt.bpp) == -1) {
		return 1;
	}

	fb_width = opt.width;
	fb_height = opt.height;
	fb_bpp = opt.bpp;
	fb_pitch = opt.width * opt.bpp / 8;

	if(!(framebuf = malloc(fb_pitch * fb_height))) {
		fprintf(stderr, "failed to allocate %dx%d (%d bpp) framebuffer\n",
				fb_width, fb_height, fb_bpp);
		return 1;
	}
	/*
	fb_rmask = ximg->red_mask;
	fb_gmask = ximg->green_mask;
	fb_bmask = ximg->blue_mask;
	fb_rshift = mask_to_shift(fb_rmask);
	fb_gshift = mask_to_shift(fb_gmask);
	fb_bshift = mask_to_shift(fb_bmask);
	*/

	if(init() == -1) {
		goto end;
	}

	/* TODO: set terminal raw and disable cursor */

	gettimeofday(&tv0, 0);

	for(;;) {
		/* TODO read input */

		gettimeofday(&tv, 0);
		time_msec = (tv.tv_sec - tv0.tv_sec) * 1000 + (tv.tv_usec - tv0.tv_usec) / 1000;
		num_frames++;

		redraw();

		/* TODO copy to fb */
	}

end:
	cleanup();
	close_fbdev();

	if(num_frames) {
		printf("avg framerate: %.1f fps\n", (10000 * num_frames / time_msec) / 10.0f);
	}
	return 0;
}

int init_fbdev(int xsz, int ysz, int bpp)
{
	if((fbfd = open("/dev/fb0", O_RDWR)) == -1) {
		perror("failed to open framebuffer device");
		return -1;
	}

	/* TODO modeset ioctl */

	fbsz = xsz * ysz * bpp / 8;	/* XXX */

	if((fbmem = mmap(0, fbsz, PROT_WRITE, MAP_SHARED, fbfd, 0)) == (void*)-1) {
		perror("failed to map framebuffer");
		return -1;
	}

	return 0;
}

void close_fbdev(void)
{
	if(fbmem) {
		munmap(fbmem, fbsz);
	}
	if(fbfd >= 0) {
		close(fbfd);
	}
}

static const char *usage_str =
	"Usage: %s [options]\n"
	"Options:\n"
	"  -s <WxH>: resolution\n"
	"  -b <bpp>: color depth\n"
	"  -h: print usage and exit\n";

static int parse_args(int argc, char **argv)
{
	int i;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2]) {
				goto inval_arg;
			}
			switch(argv[i][1]) {
			case 's':
				if(!argv[++i] || sscanf(argv[i], "%dx%d", &opt.width, &opt.height) != 2) {
					fprintf(stderr, "-s must be followed by WxH\n");
					return -1;
				}
				break;

			case 'b':
				if(!argv[++i] || !(opt.bpp = atoi(argv[i]))) {
					fprintf(stderr, "-b must be followed by the color depth\n");
					return -1;
				}
				break;

			case 'h':
				printf(usage_str, argv[0]);
				exit(0);

			default:
				goto inval_arg;
			}
		} else {
inval_arg:	fprintf(stderr, "invalid argument: %s\n", argv[i]);
			return -1;
		}
	}
	return 0;
}
