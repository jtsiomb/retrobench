#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "fbgfx.h"
#include "fbevents.h"
#include "rbench.h"
#include "util.h"

static void keyb(int key, int pressed, void *cls);
static int parse_args(int argc, char **argv);

static int quit;
static void *fbmem;

int main(int argc, char **argv)
{
	int num_frames = 0;
	struct timeval tv, tv0;
	struct fbgfx_vmode *vm;

	read_config("rbench.cfg");

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	fbgfx_save_video_mode();
	if(!(fbmem = fbgfx_set_video_mode(opt.width, opt.height, opt.bpp))) {
		fprintf(stderr, "failed to set video mode: %dx%d %dbpp\n", opt.width,
				opt.height, opt.bpp);
		return 1;
	}
	vm = fbgfx_video_mode_info();

	fb_width = vm->width;
	fb_height = vm->height;
	fb_bpp = vm->bpp;
	fb_pitch = vm->pitch;
	fb_rmask = vm->rmask;
	fb_gmask = vm->gmask;
	fb_bmask = vm->bmask;
	fb_rshift = vm->rshift;
	fb_gshift = vm->gshift;
	fb_bshift = vm->bshift;

	if(!(framebuf = malloc(fb_pitch * fb_height))) {
		fprintf(stderr, "failed to allocate %dx%d (%d bpp) framebuffer\n",
				fb_width, fb_height, fb_bpp);
		return 1;
	}

	if(fbev_init() == -1) {
		goto end;
	}
	fbev_keyboard(keyb, 0);

	if(init() == -1) {
		goto end;
	}

	gettimeofday(&tv0, 0);

	for(;;) {
		fbev_update();
		if(quit) break;

		gettimeofday(&tv, 0);
		time_msec = (tv.tv_sec - tv0.tv_sec) * 1000 + (tv.tv_usec - tv0.tv_usec) / 1000;
		num_frames++;

		redraw();
		memcpy(fbmem, framebuf, fb_pitch * fb_height);
	}

end:
	cleanup();
	fbev_shutdown();
	fbgfx_restore_video_mode();

	if(num_frames) {
		printf("avg framerate: %.1f fps\n", (10000 * num_frames / time_msec) / 10.0f);
	}
	return 0;
}

static void keyb(int key, int pressed, void *cls)
{
	if(!pressed) return;

	if(key == 27 || key == 'q' || key == 'Q') {
		quit = 1;
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
