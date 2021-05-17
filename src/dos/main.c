#include <stdio.h>
#include <stdlib.h>
#include "rbench.h"

static int parse_args(int argc, char **argv);

int main(int argc, char **argv)
{
	read_config("rbench.cfg");

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	printf("foo %dx%d %dbpp\n", opt.width, opt.height, opt.bpp);

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

