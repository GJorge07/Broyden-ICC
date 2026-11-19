#ifndef JACOBIANA_H
#define JACOBIANA_H

/* Versao nao-otimizada: Jacobiana densa n x n, J[i][j] -> J[i*n + j]. */
void jacobianaDensa(const double *x, double *J, int n);

/* Versao otimizada: Jacobiana tridiagonal guardada em tres vetores
 (subdiagonal, diagonal principal e superdiagonal), cada um com n posicoes. */

/* Preenche a sub (-1) e a superdiagonal (-2), que sao constantes. Chamar uma vez. */
void jacobianaTriConst(double *restrict sub, double *restrict sup, int n);

/* Recalcula a diagonal principal (-4x_i + 3) no ponto x. Chamar a cada iteracao. */
void jacobianaTri(const double *restrict x, double *restrict diag, int n);

#endif
