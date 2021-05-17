#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/XShm.h>
#include "rbench.h"
#include "util.h"

enum { QUIT = 1, REDRAW = 2 };

static Window create_win(int width, int height, int bpp);
static void handle_event(XEvent *ev);
static int translate_keysym(KeySym sym);
static int parse_args(int argc, char **argv);
static void sig(int s);

static int win_width, win_height;
static int mapped;
static unsigned int pending;
static Display *dpy;
static Window win, root;
static GC gc;
static Visual *vis;
static Atom xa_wm_proto, xa_wm_delwin;

static XImage *ximg;
static XShmSegmentInfo shm;
static int wait_putimg;
static int xshm_ev_completion;

int main(int argc, char **argv)
{
	int num_frames = 0;
	XEvent ev;
	struct timeval tv, tv0;

	shm.shmid = -1;
	shm.shmaddr = (void*)-1;

	signal(SIGINT, sig);

	read_config("rbench.cfg");

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	if(!(dpy = XOpenDisplay(0))) {
		fprintf(stderr, "failed to connect to the X server\n");
		return 1;
	}
	root = DefaultRootWindow(dpy);
	xa_wm_proto = XInternAtom(dpy, "WM_PROTOCOLS", 0);
	xa_wm_delwin = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);

	if(!XShmQueryExtension(dpy)) {
		fprintf(stderr, "X shared memory extension is not available\n");
		XCloseDisplay(dpy);
		return 1;
	}
	xshm_ev_completion = XShmGetEventBase(dpy) + ShmCompletion;

	if(!(win = create_win(opt.width, opt.height, opt.bpp))) {
		return 1;
	}
	gc = XCreateGC(dpy, win, 0, 0);

	if(!(ximg = XShmCreateImage(dpy, vis, opt.bpp, ZPixmap, 0, &shm, opt.width, opt.height))) {
		fprintf(stderr, "failed to create shared memory image\n");
		goto end;
	}
	if((shm.shmid = shmget(IPC_PRIVATE, ximg->bytes_per_line * ximg->height, IPC_CREAT | 0777)) == -1) {
		fprintf(stderr, "failed to create shared memory block\n");
		goto end;
	}
	if((shm.shmaddr = ximg->data = shmat(shm.shmid, 0, 0)) == (void*)-1) {
		fprintf(stderr, "failed to attach shared memory block\n");
		goto end;
	}
	shm.readOnly = True;
	if(!XShmAttach(dpy, &shm)) {
		fprintf(stderr, "XShmAttach failed");
		goto end;
	}

	fb_width = opt.width;
	fb_height = opt.height;
	if(opt.bpp >= 24) {
		fb_bpp = ximg->bytes_per_line < fb_width * 4 ? 24 : 32;
	} else {
		fb_bpp = opt.bpp;
	}
	framebuf = ximg->data;
	fb_pitch = ximg->bytes_per_line;
	fb_rmask = ximg->red_mask;
	fb_gmask = ximg->green_mask;
	fb_bmask = ximg->blue_mask;
	fb_rshift = mask_to_shift(fb_rmask);
	fb_gshift = mask_to_shift(fb_gmask);
	fb_bshift = mask_to_shift(fb_bmask);

	if(init() == -1) {
		goto end;
	}

	gettimeofday(&tv0, 0);

	while(!(pending & QUIT)) {
		if(mapped) {/* && !wait_putimg) { */
			while(XPending(dpy)) {
				XNextEvent(dpy, &ev);
				handle_event(&ev);
				if(pending & QUIT) goto end;
			}

			if(!wait_putimg) {
				gettimeofday(&tv, 0);
				time_msec = (tv.tv_sec - tv0.tv_sec) * 1000 + (tv.tv_usec - tv0.tv_usec) / 1000;
				num_frames++;

				redraw();

				XShmPutImage(dpy, win, gc, ximg, 0, 0, 0, 0, ximg->width, ximg->height, True);
				wait_putimg = 1;
			}
		} else {
			XNextEvent(dpy, &ev);
			handle_event(&ev);
			if(pending & QUIT) goto end;
		}
	}

end:
	cleanup();
	if(ximg) {
		XShmDetach(dpy, &shm);
		XDestroyImage(ximg);
		if(shm.shmaddr != (void*)-1) {
			shmdt(shm.shmaddr);
		}
		if(shm.shmid != -1) {
			shmctl(shm.shmid, IPC_RMID, 0);
		}
	}
	if(win) {
		XFreeGC(dpy, gc);
		XDestroyWindow(dpy, win);
	}
	XCloseDisplay(dpy);

	if(num_frames) {
		printf("avg framerate: %.1f fps\n", (10000 * num_frames / time_msec) / 10.0f);
	}
	return 0;
}

static Window create_win(int width, int height, int bpp)
{
	int scr, num_vis;
	Window win;
	XVisualInfo *vinf, vtmpl;
	unsigned int vinf_mask;
	XSetWindowAttributes xattr;
	XTextProperty txname;
	Colormap cmap;
	const char *name = "retrobench X11";

	scr = DefaultScreen(dpy);

	vtmpl.screen = scr;
	vtmpl.depth = bpp;
	vtmpl.class = bpp <= 8 ? PseudoColor : TrueColor;
	vinf_mask = VisualScreenMask | VisualDepthMask | VisualClassMask;
	if(!(vinf = XGetVisualInfo(dpy, vinf_mask, &vtmpl, &num_vis))) {
		fprintf(stderr, "failed to find appropriate visual for %d bpp\n", bpp);
		return 0;
	}
	vis = vinf->visual;

	if(!(cmap = XCreateColormap(dpy, root, vis, bpp <= 8 ? AllocAll : AllocNone))) {
		fprintf(stderr, "failed to allocate colormap\n");
		return 0;
	}

	xattr.background_pixel = BlackPixel(dpy, scr);
	xattr.colormap = cmap;
	xattr.override_redirect = True;
	win = XCreateWindow(dpy, root, 0, 0, width, height, 0, vinf->depth,
			InputOutput, vis, CWColormap | CWBackPixel | CWOverrideRedirect, &xattr);
	if(!win) return 0;

	XSelectInput(dpy, win, StructureNotifyMask | ExposureMask | KeyPressMask |
			KeyReleaseMask);

	XStringListToTextProperty((char**)&name, 1, &txname);
	XSetWMName(dpy, win, &txname);
	XSetWMIconName(dpy, win, &txname);
	XFree(txname.value);

	XSetWMProtocols(dpy, win, &xa_wm_delwin, 1);

	XMapWindow(dpy, win);
	return win;
}

static void handle_event(XEvent *ev)
{
	int key;
	KeySym sym;

	switch(ev->type) {
	case MapNotify:
		mapped = 1;
		break;

	case UnmapNotify:
		mapped = 0;
		break;

	case Expose:
		pending |= REDRAW;
		break;

	case ConfigureNotify:
		if(ev->xconfigure.width != win_width || ev->xconfigure.height != win_height) {
			win_width = ev->xconfigure.width;
			win_height = ev->xconfigure.height;
			/* TODO */
		}
		break;

	case KeyPress:
	case KeyRelease:
		if((sym = XKeycodeToKeysym(dpy, ev->xkey.keycode, 0))) {
			if(sym == XK_Escape) {
				pending |= QUIT;
				break;
			}
			if((key = translate_keysym(sym))) {
				key_event(key, ev->xkey.type == KeyPress);
			}
		}
		break;

	case ClientMessage:
		if(ev->xclient.message_type == xa_wm_proto) {
			if(ev->xclient.data.l[0] == xa_wm_delwin) {
				pending |= QUIT;
			}
		}
		break;

	default:
		if(ev->type == xshm_ev_completion) {
			wait_putimg = 0;
		}
		break;
	}
}

static int translate_keysym(KeySym sym)
{
	if(sym < 128) return sym;

	switch(sym) {
	case XK_Escape:
		return 27;

	default:
		break;
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

static void sig(int s)
{
	pending |= QUIT;
}
