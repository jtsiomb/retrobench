#ifndef LOGGER_H_
#define LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

int init_logger(const char *fname);
void stop_logger(void);

#ifdef __cplusplus
}
#endif

#endif	/* LOGGER_H_ */
