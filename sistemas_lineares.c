#include <math.h>
#include "sistemas_lineares.h"

/* Troca as linhas k e p de A (a partir da coluna k) e de b. */
static void trocaLinhas(double *A, double *b, int n, int k, int p)
{
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
 * Propositalmente ingenua: nao explora a estrutura tridiagonal, ou seja,
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
