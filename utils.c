#include <time.h>
#include "utils.h"

/* CLOCK_MONOTONIC_RAW nao existe no Windows/MinGW; usa CLOCK_MONOTONIC la. */
#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif

/* Retorna um instante de tempo em milissegundos. */
rtime_t timestamp(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    return (rtime_t) tp.tv_sec * 1.0e3 + (rtime_t) tp.tv_nsec * 1.0e-6;
}
