#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "rbench.h"

enum { QUIT = 1, REDRAW = 2 };

static Window create_win(int width, int height, int bpp);
static void handle_event(XEvent *ev);
static int translate_keysym(KeySym sym);
static int parse_args(int argc, char **argv);

static int win_width, win_height;
static int mapped;
static unsigned int pending;
static Display *dpy;
static Window win, root;
static Atom xa_wm_proto, xa_wm_delwin;

int main(int argc, char **argv)
{
	XEvent ev;

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

	if(!(win = create_win(opt.width, opt.height, opt.bpp))) {
		return 1;
	}

	for(;;) {
		if(mapped) {
			while(XPending(dpy)) {
				XNextEvent(dpy, &ev);
				handle_event(&ev);
				if(pending & QUIT) goto end;
			}
			redraw();
		} else {
			XNextEvent(dpy, &ev);
			handle_event(&ev);
			if(pending & QUIT) goto end;
		}
	}

end:
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
	return 0;
}

static Window create_win(int width, int height, int bpp)
{
	int scr, num_vis;
	Window win;
	XVisualInfo *vis, vinf;
	unsigned int vinf_mask;
	XSetWindowAttributes xattr;
	XTextProperty txname;
	Colormap cmap;
	const char *name = "retrobench X11";

	scr = DefaultScreen(dpy);

	vinf.screen = scr;
	vinf.depth = bpp;
	vinf.class = bpp <= 8 ? PseudoColor : TrueColor;
	vinf_mask = VisualScreenMask | VisualDepthMask | VisualClassMask;
	if(!(vis = XGetVisualInfo(dpy, vinf_mask, &vinf, &num_vis))) {
		fprintf(stderr, "failed to find appropriate visual for %d bpp\n", bpp);
		return 0;
	}

	if(!(cmap = XCreateColormap(dpy, root, vis->visual, bpp <= 8 ? AllocAll : AllocNone))) {
		fprintf(stderr, "failed to allocate colormap\n");
		return 0;
	}

	xattr.background_pixel = BlackPixel(dpy, scr);
	xattr.colormap = cmap;
	win = XCreateWindow(dpy, root, 0, 0, width, height, 0, vis->depth,
			InputOutput, vis->visual, CWColormap | CWBackPixel, &xattr);
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
