#ifndef NEWTON_H
#define NEWTON_H

#include "utils.h"

/* Tempos acumulados (em ms) medidos durante o Metodo de Newton. */
typedef struct {
    rtime_t total;      /* execucao total do metodo                 */
    rtime_t jacobiana;  /* soma dos calculos de Jacobiana           */
    rtime_t sl;         /* soma das resolucoes de sistemas lineares */
} TemposNewton;

/*
 * Aplica o Metodo de Newton a F(x) = 0 partindo de x (tamanho n).
 * Para quando ||F(x)||_inf < eps, ||delta||_inf < eps ou apos maxIt iteracoes.
 * hist deve ter espaco para (maxIt+1)*n doubles: o iterado k fica em
 * hist[k*n .. k*n+n-1] (k = 0 e o chute inicial).
 * Retorna o numero de iteracoes efetuadas (ou -1 em caso de erro).
 * Ao final, x contem a ultima aproximacao.
 */
int newton(double *x, int n, double eps, int maxIt, double *hist, TemposNewton *t);

#endif
