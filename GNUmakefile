src = $(wildcard src/*.c) $(wildcard src/x11/*.c)
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = rbench

warn = -pedantic -Wall -Wno-deprecated-declarations
dbg = -g
#opt = -O3 -ffast-math
inc = -Isrc

CFLAGS = -pedantic $(warn) $(dbg) $(opt) $(inc) -fcommon -MMD
LDFLAGS = -lX11 -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	$(RM) $(obj) $(bin)

.PHONY: cleandep
cleandep:
	$(RM) $(dep)
