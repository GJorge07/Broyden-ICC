#include <stdio.h>
#include <stdlib.h>
#include "newton.h"
#include "likwid_markers.h"

/*
 * Le N, x0, epsilon e MAX da entrada padrao, aplica o Metodo de Newton ao
 * SNLB e imprime todas as iteracoes seguidas dos tempos (ms).
 */
int main(void)
{
    int n, maxIt;
    double x0, eps;

    if (scanf("%d %lf %lf %d", &n, &x0, &eps, &maxIt) != 4 || n < 2 || maxIt < 0) {
        fprintf(stderr, "Entrada invalida. Esperado: N(>=2) x0 epsilon MAX\n");
        return 1;
    }

    double *x = malloc((size_t) n * sizeof(double));
    double *hist = malloc((size_t) (maxIt + 1) * n * sizeof(double));
    if (!x || !hist) {
        fprintf(stderr, "Erro: memoria insuficiente\n");
        return 1;
    }
    for (int i = 0; i < n; ++i)
        x[i] = x0;

    LIKWID_MARKER_INIT;

    TemposNewton t;
    int it = newton(x, n, eps, maxIt, hist, &t);

    LIKWID_MARKER_CLOSE;

    if (it < 0) {
        free(x);
        free(hist);
        return 1;
    }

    /* Impressao fica fora da regiao medida: so e feita apos o metodo terminar */
    for (int k = 0; k <= it; ++k) {
        if (k > 0)
            printf("#\n");
        const double *xk = hist + (size_t) k * n;
        for (int i = 0; i < n; ++i)
            printf("x%d = %.15g\n", i + 1, xk[i]);
    }

    printf("###########\n");
    printf("%.8lf\n%.8lf\n%.8lf\n", t.total, t.jacobiana, t.sl);
    printf("###########\n");

    free(x);
    free(hist);
    return 0;
}
