#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "rbench.h"
#include "timer.h"
#include "gfx.h"
#include "logger.h"
#include "cdpmi.h"

static int parse_args(int argc, char **argv);

static struct video_mode *vidmode;

int main(int argc, char **argv)
{
	int vmidx;
	int num_frames = 0;
	void *vmem;

#ifdef __DJGPP__
	__djgpp_nearptr_enable();
#endif

	read_config("rbench.cfg");

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	init_logger("rbench.log");

	if(init_video() == -1) {
		return 1;
	}

	if((vmidx = match_video_mode(opt.width, opt.height, opt.bpp)) == -1) {
		return 1;
	}
	if(!(vmem = set_video_mode(vmidx, 1))) {
		return 1;
	}
	vidmode = get_video_mode(vmidx);

	fb_rmask = vidmode->rmask;
	fb_gmask = vidmode->gmask;
	fb_bmask = vidmode->bmask;
	fb_rshift = vidmode->rshift;
	fb_gshift = vidmode->gshift;
	fb_bshift = vidmode->bshift;

	init_timer(100);

	for(;;) {
		while(kbhit()) {
			int c = getch();
			if(c == 27) goto end;
			key_event(c, 1);
		}

		time_msec = get_msec();
		num_frames++;
		redraw();

		blit_frame(framebuf, 0);
	}

end:
	set_text_mode();
	cleanup_video();
	stop_logger();

	if(num_frames) {
		printf("%d frames in %d msec\n", num_frames, time_msec);
		printf("avg framerate: %.1f fps\n", (10000 * num_frames / time_msec) / 10.0f);
	}
	return 0;
}

int resizefb(int x, int y, int bpp, int pitch)
{
	printf("resizefb %dx%d %dbpp (pitch: %d)\n", x, y, bpp, pitch);

	free(framebuf);

	fb_width = x;
	fb_height = y;
	fb_bpp = bpp;
	fb_pitch = pitch;

	if(!(framebuf = malloc(fb_pitch * fb_height))) {
		fprintf(stderr, "failed to allocate %dx%d (%dbpp) framebuffer\n",
				fb_width, fb_height, fb_bpp);
		return -1;
	}
	return 0;
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

