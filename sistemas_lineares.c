#include <math.h>
#include "sistemas_lineares.h"

/* Troca as linhas k e p de A (a partir da coluna k) e de b. */
static void trocaLinhas(double *A, double *b, int n, int k, int p) {
    
    for (int j = k; j < n; ++j) {
        double t = A[k * n + j];
        A[k * n + j] = A[p * n + j];
        A[p * n + j] = t;
    }
    double t = b[k];
    b[k] = b[p];
    b[p] = t;
}

/* Eliminacao de Gauss com pivoteamento parcial + retrossubstituicao. O(n^3).
 * Propositalmente nao otimizada: nao explora a estrutura tridiagonal, ou seja,
 * opera sobre todos os zeros da matriz. */
int gaussDenso(double *A, double *b, int n)
{
    for (int k = 0; k < n; ++k) {
        /* Pivoteamento parcial: maior |A[i][k]| para i >= k */
        int p = k;
        double max = fabs(A[k * n + k]);
        for (int i = k + 1; i < n; ++i) {
            if (fabs(A[i * n + k]) > max) {
                max = fabs(A[i * n + k]);
                p = i;
            }
        }
        if (max == 0.0)
            return -1;
        if (p != k)
            trocaLinhas(A, b, n, k, p);

        /* Zera a coluna k abaixo da diagonal */
        for (int i = k + 1; i < n; ++i) {
            double m = A[i * n + k] / A[k * n + k];
            A[i * n + k] = 0.0;
            for (int j = k + 1; j < n; ++j)
                A[i * n + j] -= m * A[k * n + j];
            b[i] -= m * b[k];
        }
    }

    /* Retrossubstituicao */
    for (int i = n - 1; i >= 0; --i) {
        double s = b[i];
        for (int j = i + 1; j < n; ++j)
            s -= A[i * n + j] * b[j];
        b[i] = s / A[i * n + i];
    }
    return 0;
}


/* Eliminacao de Gauss para sistemas tridiagonais (versao otimizada).
 * Nao faz pivoteamento: assim nao surge preenchimento fora das 3 diagonais.
 * Altera apenas diag e b; a solucao fica em b. */
void gaussTridiagonal(const double *restrict sub, double *restrict diag,
                      const double *restrict sup, double *restrict b, int n)
{
    /* Triangularizacao  */
    for (int i = 0; i < n - 1; ++i) {
        double inv = 1.0 / diag[i];
        diag[i] = inv;
        double m = sub[i + 1] * inv;
        diag[i + 1] -= m * sup[i];
        b[i + 1]    -= m * b[i];
    }

    /* Retrossubstituicao no proprio b*/
    b[n - 1] /= diag[n - 1];
    for (int i = n - 2; i >= 0; --i)
        b[i] = (b[i] - sup[i] * b[i + 1]) * diag[i];
}
