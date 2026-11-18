#ifndef UTILS_H
#define UTILS_H

/*
 * Medicao de tempo (wall-time) com relogio monotonico.
 * timestamp() devolve o tempo atual em milissegundos.
 */
typedef double rtime_t;

/* Retorna um instante de tempo em milissegundos (relogio monotonico). */
rtime_t timestamp(void);

#endif
