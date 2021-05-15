#include <stdio.h>
#include <stdlib.h>
#include "rbench.h"
#include "treestor.h"

#define DEF_WIDTH	640
#define DEF_HEIGHT	480
#define DEF_BPP		24

struct options opt = {
	DEF_WIDTH, DEF_HEIGHT, DEF_BPP
};

void redraw(void)
{
}

void key_event(int key, int press)
{
}

int read_config(const char *fname)
{
	FILE *fp;
	struct ts_node *ts;

	if(!(fp = fopen(fname, "rb"))) {
		return -1;
	}
	fclose(fp);

	if(!(ts = ts_load(fname))) {
		return -1;
	}

	opt.width = ts_lookup_int(ts, "rbench.width", DEF_WIDTH);
	opt.height = ts_lookup_int(ts, "rbench.height", DEF_HEIGHT);
	opt.bpp = ts_lookup_int(ts, "rbench.bpp", DEF_BPP);

	ts_free_tree(ts);
	return 0;
}
