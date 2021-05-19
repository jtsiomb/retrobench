#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "fbevents.h"

static struct termios orig_state;
static int term_fd = -1;	/* tty file descriptor */

static void (*keyb_func)(int, int, void*);
static void *keyb_cls;
static void (*mbutton_func)(int, int, int, int, void*);
static void *mbutton_cls;
static void (*mmotion_func)(int, int, void*);
static void *mmotion_cls;


int fbev_init(void)
{
	struct termios tstate;

	if((term_fd = open("/dev/tty", O_RDWR)) == -1) {
		perror("failed to open tty");
		return -1;
	}
	fcntl(term_fd, F_SETFL, fcntl(term_fd, F_GETFL) | O_NONBLOCK);

	if(tcgetattr(term_fd, &tstate) == -1) {
		perror("failed to retrieve tty attribs");
		close(term_fd);
		return -1;
	}
	orig_state = tstate;

	cfmakeraw(&tstate);
	/* keep output flags as they where, since we'll be drawing directly to fbdev */
	tstate.c_oflag = orig_state.c_oflag;
	if(tcsetattr(term_fd, TCSANOW, &tstate) == -1) {
		perror("failed put terminal into raw mode");
		close(term_fd);
		return -1;
	}
	return 0;
}

void fbev_shutdown(void)
{
	if(term_fd >= 0) {
		tcsetattr(term_fd, TCSANOW, &orig_state);
		close(term_fd);
		term_fd = -1;
	}
}

void fbev_update(void)
{
	char buf[64];
	int i, sz;
	assert(term_fd >= 0);

	while((sz = read(term_fd, buf, 64)) > 0) {
		if(keyb_func) {
			for(i=0; i<sz; i++) {
				keyb_func(buf[i], 1, keyb_cls);
			}
		}
	}
}

void fbev_keyboard(void (*func)(int, int, void*), void *cls)
{
	keyb_func = func;
	keyb_cls = cls;
}

void fbev_mbutton(void (*func)(int, int, int, int, void*), void *cls)
{
	mbutton_func = func;
	mbutton_cls = cls;
}

void fbev_mmotion(void (*func)(int, int, void*), void *cls)
{
	mmotion_func = func;
	mmotion_cls = cls;
}
