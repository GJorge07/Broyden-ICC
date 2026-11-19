#ifndef BROYDEN_H
#define BROYDEN_H

/*
 * Avalia o Sistema Nao-Linear de Broyden F(x) no ponto x.
 *   f_1(x) = -2x_1^2 + 3x_1 - 2x_2 + 1
 *   f_i(x) = -2x_i^2 + 3x_i - x_{i-1} - 2x_{i+1} + 1,  2 <= i <= n-1
 *   f_n(x) = -2x_n^2 + 3x_n - x_{n-1}
 * x e F sao vetores de tamanho n (n >= 2). Indices em C comecam em 0.
 */
void avaliaF(const double *x, double *F, int n);

/* Retorna a norma do maximo ||v||_inf = max |v_i|. */
double normaMax(const double *v, int n);

#endif
