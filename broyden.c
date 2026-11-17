#include <math.h>
#include "broyden.h"

/* Avalia F(x). Primeira e ultima equacoes tratadas fora do laco (loop peeling),
 * assim o corpo do laco nao tem desvios condicionais. */
void avaliaF(const double *x, double *F, int n)
{
    F[0] = -2.0 * x[0] * x[0] + 3.0 * x[0] - 2.0 * x[1] + 1.0;

    for (int i = 1; i < n - 1; ++i)
        F[i] = -2.0 * x[i] * x[i] + 3.0 * x[i] - x[i - 1] - 2.0 * x[i + 1] + 1.0;

    F[n - 1] = -2.0 * x[n - 1] * x[n - 1] + 3.0 * x[n - 1] - x[n - 2];
}

/* Norma do maximo de um vetor. */
double normaMax(const double *v, int n)
{
    double max = 0.0;
    for (int i = 0; i < n; ++i) {
        double a = fabs(v[i]);
        if (a > max)
            max = a;
    }
    return max;
}
