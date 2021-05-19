src = $(wildcard src/*.c)
src_x11 = $(wildcard src/x11/*.c)
src_fbdev = $(wildcard src/fbdev/*.c)
ssrc = sinlut.s
obj_x11 = $(src:.c=.o) $(src_x11:.c=.o) $(ssrc:.s=.o)
obj_fbdev = $(src:.c=.o) $(src_fbdev:.c=.o) $(ssrc:.s=.o)
dep = $(src:.c=.d) $(src_x11:.c=.d) $(src_fbdev:.c=.d)
bin_x11 = rbench_x11
bin_fbdev = rbench_fbdev

warn = -pedantic -Wall -Wno-deprecated-declarations
dbg = -g
opt = -O3 -ffast-math
inc = -Isrc

CFLAGS = -pedantic $(warn) $(dbg) $(opt) $(inc) -fno-strict-aliasing -MMD
LDFLAGS_x11 = -L/usr/X11R6/lib -lX11 -lXext
LDFLAGS_fbdev =

.PHONY: all
all: $(bin_x11) $(bin_fbdev)

$(bin_x11): $(obj_x11)
	$(CC) -o $@ $(obj_x11) $(LDFLAGS_x11)

$(bin_fbdev): $(obj_fbdev)
	$(CC) -o $@ $(obj_fbdev) $(LDFLAGS_fbdev)

sinlut.s: tools/lutgen
	tools/lutgen >$@

-include $(dep)

.PHONY: clean
clean:
	$(RM) $(obj_x11) $(obj_fbdev) $(bin_x11) $(bin_fbdev)

.PHONY: cleandep
cleandep:
	$(RM) $(dep)

tools/lutgen: tools/lutgen.c
	$(CC) -o $@ $< -lm
