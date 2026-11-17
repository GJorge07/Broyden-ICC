#include <stdio.h>
#include "newton.h"

/*
 * VERSAO OTIMIZADA do Metodo de Newton (Jacobiana tridiagonal + Thomas).
 * Ainda nao implementada: sera feita pelo Gabriel (Newton/Jacobiana)
 * usando o solver tridiagonal do Arthur.
 */
int newton(double *x, int n, double eps, int maxIt, double *hist, TemposNewton *t)
{
    (void) x; (void) n; (void) eps; (void) maxIt; (void) hist; (void) t;
    fprintf(stderr, "Versao otimizada ainda nao implementada. Use ./broyden-ingenuo\n");
    return -1;
}
