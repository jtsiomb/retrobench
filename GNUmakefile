src = $(wildcard src/*.c) $(wildcard src/x11/*.c)
ssrc = sinlut.s
obj = $(src:.c=.o) $(ssrc:.s=.o)
dep = $(src:.c=.d)
bin = rbench

warn = -pedantic -Wall -Wno-deprecated-declarations
dbg = -g
opt = -O3 -ffast-math
inc = -Isrc

CFLAGS = -pedantic $(warn) $(dbg) $(opt) $(inc) -MMD
LDFLAGS = -L/usr/X11R6/lib -lX11 -lXext -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

sinlut.s: tools/lutgen
	tools/lutgen >$@

-include $(dep)

.PHONY: clean
clean:
	$(RM) $(obj) $(bin)

.PHONY: cleandep
cleandep:
	$(RM) $(dep)

tools/lutgen: tools/lutgen.c
	$(CC) -o $@ $< -lm
