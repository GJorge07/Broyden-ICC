#ifndef UTILS_H
#define UTILS_H

/*
 * PROVISORIO: substituir pelos utils.h/utils.c fornecidos pelo professor.
 * Mantem a mesma interface: timestamp() devolve o tempo atual em milissegundos.
 */
typedef double rtime_t;

/* Retorna um instante de tempo em milissegundos (relogio monotonico). */
rtime_t timestamp(void);

#endif
