#ifndef JACOBIANA_H
#define JACOBIANA_H

/*
 * VERSAO INGENUA: calcula a matriz Jacobiana J(x) completa (densa, n x n),
 * armazenada por linhas em um vetor contiguo: J[i][j] -> J[i*n + j].
 */
void jacobianaDensa(const double *x, double *J, int n);

#endif
