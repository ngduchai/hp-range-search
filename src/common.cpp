
#include "common.h"
#include <sys/time.h>

time_t time_us(void) {
	timeval tp;
	gettimeofday(&tp, NULL);
	return (time_t)tp.tv_sec * 1000000  + (time_t)tp.tv_usec;
}


