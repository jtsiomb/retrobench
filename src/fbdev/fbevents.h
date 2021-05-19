#ifndef FBEVENTS_H_
#define FBEVENTS_H_

int fbev_init(void);
void fbev_shutdown(void);

void fbev_update(void);

void fbev_keyboard(void (*func)(int, int, void*), void *cls);
void fbev_mbutton(void (*func)(int, int, int, int, void*), void *cls);
void fbev_mmotion(void (*func)(int, int, void*), void *cls);

#endif	/* FBEVENTS_H_ */
