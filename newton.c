#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#include <malloc.h>   /* _aligned_malloc (so para testes no Windows) */
#endif
#include "newton.h"
#include "broyden.h"
#include "jacobiana.h"
#include "sistemas_lineares.h"
#include "likwid_markers.h"

/*Avaliacao de -F(x) otimizada (a versao nao otimizada usa avaliaF de broyden.c
 e depois troca o sinal em outro laco):
 - Faca menos trabalho: o sistema linear precisa de -F, entao -F e calculado
   direto, sem um segundo laco so para trocar o sinal (uma passada a menos de
   leitura e escrita em memoria). Como o sinal ja entra na formula, o
   resultado e exatamente o mesmo de calcular F e trocar o sinal depois.
 - Eliminacao de subexpressoes / reduzir operacoes: 2x^2 - 3x foi fatorado
   como x*(2x - 3), uma multiplicacao a menos por equacao.
 - restrict: x e menosF nao se sobrepoem, entao o GCC vetoriza sem gerar a
   versao alternativa "versioned for aliasing" que aparece em avaliaF.
 - Loop peeling: primeira e ultima equacao fora do laco (laco sem if).
 - static inline: chamada eliminada, o corpo e copiado dentro do newton().*/
static inline void avaliaMenosF(const double *restrict x, double *restrict menosF, int n) {

    menosF[0] = x[0] * (2.0 * x[0] - 3.0) + 2.0 * x[1] - 1.0;

    for (int i = 1; i < n - 1; ++i)
        menosF[i] = x[i] * (2.0 * x[i] - 3.0) + x[i - 1] + 2.0 * x[i + 1] - 1.0;

    menosF[n - 1] = x[n - 1] * (2.0 * x[n - 1] - 3.0) + x[n - 2];

}

/*Fusao de lacos: atualiza x = x + delta, copia o novo x para o historico e
 calcula ||delta||_inf, tudo na mesma passada (em vez de tres lacos: soma,
 memcpy e normaMax).*/
static inline double atualizaX(double *restrict x, const double *restrict delta,
    double *restrict histK, int n) {

    double max = 0.0;
    for (int i = 0; i < n; ++i) {

        double a = fabs(delta[i]);
        max = (a > max) ? a : max;
        x[i] += delta[i];
        histK[i] = x[i];

    }
    return max;

}

/*Aloca um vetor de n doubles alinhado em 32 bytes (tamanho de um registrador
 AVX), para as instrucoes SIMD acessarem a memoria alinhada.
 aligned_alloc exige tamanho multiplo do alinhamento, por isso o arredondamento.
 No Windows (so para testes) nao existe aligned_alloc: usa _aligned_malloc.*/
static double *alocaVetor(int n) {

    size_t bytes = ((size_t) n * sizeof(double) + 31) / 32 * 32;
#ifdef _WIN32
    return _aligned_malloc(bytes, 32);
#else
    return aligned_alloc(32, bytes);
#endif

}

/*Libera um vetor alocado com alocaVetor.*/
static void liberaVetor(double *v) {

#ifdef _WIN32
    _aligned_free(v);
#else
    free(v);
#endif

}

/*
 * VERSAO OTIMIZADA do Metodo de Newton:
 * Jacobiana tridiagonal (3 vetores), so a diagonal recalculada a cada
 * iteracao, O(n), e sistema linear resolvido por Eliminacao de Gauss
 * tridiagonal, O(n).
 * Mesma interface e mesmos criterios de parada da versao nao otimizada.
 */
int newton(double *x, int n, double eps, int maxIt, double *hist, TemposNewton *t) {

    /* Todos os vetores alocados uma unica vez, fora do laco, alinhados */
    double *F    = alocaVetor(n);
    double *sub  = alocaVetor(n);
    double *diag = alocaVetor(n);
    double *sup  = alocaVetor(n);

    if (!F || !sub || !diag || !sup) {

        fprintf(stderr, "Erro: memoria insuficiente\n");
        liberaVetor(F); liberaVetor(sub); liberaVetor(diag); liberaVetor(sup);
        return -1;

    }

    t->total = t->jacobiana = t->sl = 0.0;
    memcpy(hist, x, (size_t) n * sizeof(double)); /* iteracao 0 */

    int it = 0;

    /* Os marcadores LIKWID ficam por FORA dos timestamp(), para o custo
     deles nao entrar nos tempos medidos */
    LIKWID_MARKER_START("newton");
    rtime_t t0 = timestamp();

    /* Parte constante da Jacobiana: calculada uma unica vez, fora do laco
     (o tempo e as metricas entram no trecho da Jacobiana) */
    LIKWID_MARKER_START("jacobiana");
    rtime_t tj = timestamp();
    jacobianaTriConst(sub, sup, n);
    t->jacobiana += timestamp() - tj;
    LIKWID_MARKER_STOP("jacobiana");

    for (int k = 0; k < maxIt; ++k) {

        /* F recebe -F(x) direto; ||-F||_inf = ||F||_inf */
        avaliaMenosF(x, F, n);
        if (normaMax(F, n) < eps)
            break;

        LIKWID_MARKER_START("jacobiana");
        tj = timestamp();
        jacobianaTri(x, diag, n);
        t->jacobiana += timestamp() - tj;
        LIKWID_MARKER_STOP("jacobiana");

        /* Sistema J * delta = -F; delta e devolvido em F */
        LIKWID_MARKER_START("sl");
        rtime_t ts = timestamp();
        gaussTridiagonal(sub, diag, sup, F, n);
        t->sl += timestamp() - ts;
        LIKWID_MARKER_STOP("sl");

        /* x = x + delta, guarda no historico e ||delta||_inf, em uma passada */
        it = k + 1;
        double normaDelta = atualizaX(x, F, hist + (size_t) it * n, n);

        if (normaDelta < eps)
            break;

    }

    t->total = timestamp() - t0;
    LIKWID_MARKER_STOP("newton");

    liberaVetor(F); liberaVetor(sub); liberaVetor(diag); liberaVetor(sup);
    return it;
}
