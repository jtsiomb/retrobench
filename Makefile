src = $(wildcard src/*.c)
ssrc = $(wildcard src/*.s) sinlut.s
src_x11 = $(wildcard src/x11/*.c)
src_fbdev = $(wildcard src/fbdev/*.c)
obj_x11 = $(src_x11:.c=.o) $(src:.c=.o) $(ssrc:.s=.o)
obj_fbdev = $(src_fbdev:.c=.o) $(src:.c=.o) $(ssrc:.s=.o)
dep = $(src:.c=.d) $(src_x11:.c=.d) $(src_fbdev:.c=.d)
bin_x11 = rbench_x11
bin_fbdev = rbench_fbdev

warn = -pedantic -Wall -Wno-deprecated-declarations
dbg = -g
opt = -O3 -ffast-math
inc = -Isrc

CFLAGS = $(ccarch) -pedantic $(warn) $(dbg) $(opt) $(inc) -fno-strict-aliasing -MMD
ASFLAGS = $(asarch)
LDFLAGS_x11 = -L/usr/X11R6/lib -lX11 -lXext
LDFLAGS_fbdev =

ifeq ($(shell uname -m), x86_64)
	ccarch = -m32
	asarch = --32
endif

.PHONY: all
all: $(bin_x11) $(bin_fbdev)

$(bin_x11): $(obj_x11)
	$(CC) -o $@ $(ccarch) $(obj_x11) $(LDFLAGS_x11)

$(bin_fbdev): $(obj_fbdev)
	$(CC) -o $@ $(ccarch) $(obj_fbdev) $(LDFLAGS_fbdev)

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
