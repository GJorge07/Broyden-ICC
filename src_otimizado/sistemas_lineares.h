#ifndef SISTEMAS_LINEARES_H
#define SISTEMAS_LINEARES_H

/*
 * VERSAO NAO OTIMIZADA: Eliminacao de Gauss com pivoteamento parcial em matriz
 * densa n x n (armazenada por linhas). Resolve A x = b in-place:
 * A e destruida e a solucao x e devolvida em b.
 * Retorna 0 em caso de sucesso, -1 se a matriz for singular.
 */
int gaussDenso(double *A, double *b, int n);

/*
 * VERSAO OTIMIZADA: Eliminacao de Gauss para sistemas tridiagonais, sem
 * operar sobre os zeros da matriz, O(n). Linha i do sistema:
 *   sub[i]*x[i-1] + diag[i]*x[i] + sup[i]*x[i+1] = b[i]
 * diag e b sao destruidos; a solucao e devolvida em b.
 * Nao faz pivoteamento (mantem a estrutura tridiagonal).
 */
void gaussTridiagonal(const double *restrict sub, double *restrict diag,
                      const double *restrict sup, double *restrict b, int n);

#endif
