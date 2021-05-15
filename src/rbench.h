#ifndef RETROBENCH_H_
#define RETROBENCH_H_

struct options {
	int width, height, bpp;
};

extern struct options opt;

void redraw(void);
void key_event(int key, int press);

int read_config(const char *fname);


#endif	/* RETROBENCH_H_ */
