#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "newton.h"
#include "broyden.h"
#include "jacobiana.h"
#include "sistemas_lineares.h"
#include "likwid_markers.h"

/*
 * VERSAO NAO OTIMIZADA (baseline sem otimizacao) do Metodo de Newton:
 * Jacobiana densa n x n recalculada a cada iteracao + Gauss denso O(n^3).
 */
int newton(double *x, int n, double eps, int maxIt, double *hist, TemposNewton *t) {

    double *F = malloc((size_t) n * sizeof(double));
    double *J = malloc((size_t) n * n * sizeof(double));

    if (!F || !J) {

        fprintf(stderr, "Erro: memoria insuficiente para Jacobiana %d x %d\n", n, n);
        free(F);
        free(J);
        return -1;

    }

    t->total = t->jacobiana = t->sl = 0.0;
    memcpy(hist, x, (size_t) n * sizeof(double)); /* iteracao 0 */

    int it = 0;

    /* Os marcadores LIKWID ficam por FORA dos timestamp(), para o custo
     deles nao entrar nos tempos medidos */
    LIKWID_MARKER_START("newton");
    rtime_t t0 = timestamp();

    for (int k = 0; k < maxIt; ++k) {

        avaliaF(x, F, n);
        if (normaMax(F, n) < eps)
            break;

        LIKWID_MARKER_START("jacobiana");
        rtime_t tj = timestamp();
        jacobianaDensa(x, J, n);
        t->jacobiana += timestamp() - tj;
        LIKWID_MARKER_STOP("jacobiana");

        /* Sistema J * delta = -F; delta e devolvido em F */
        for (int i = 0; i < n; ++i)
            F[i] = -F[i];

        LIKWID_MARKER_START("sl");
        rtime_t ts = timestamp();
        int erro = gaussDenso(J, F, n);
        t->sl += timestamp() - ts;
        LIKWID_MARKER_STOP("sl");

        if (erro) {

            fprintf(stderr, "Erro: Jacobiana singular na iteracao %d\n", k);
            break;

        }

        for (int i = 0; i < n; ++i)
            x[i] += F[i];

        it = k + 1;
        memcpy(hist + (size_t) it * n, x, (size_t) n * sizeof(double));

        if (normaMax(F, n) < eps)
            break;
    }

    t->total = timestamp() - t0;
    LIKWID_MARKER_STOP("newton");

    free(F);
    free(J);
    return it;
}
