#ifndef SISTEMAS_LINEARES_H
#define SISTEMAS_LINEARES_H

/*
 * VERSAO INGENUA: Eliminacao de Gauss com pivoteamento parcial em matriz
 * densa n x n (armazenada por linhas). Resolve A x = b in-place:
 * A e destruida e a solucao x e devolvida em b.
 * Retorna 0 em caso de sucesso, -1 se a matriz for singular.
 */
int gaussDenso(double *A, double *b, int n);

#endif
