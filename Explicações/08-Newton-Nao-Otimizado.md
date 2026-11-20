# 08. `newton_nao_otimizado.c`: a versão de referência

A **baseline**: o Newton escrito do jeito mais direto, sem explorar a estrutura do problema.
Serve de comparação nos gráficos.

```c
double *F = malloc(n * sizeof(double));
double *J = malloc(n * n * sizeof(double));       // matriz densa
memcpy(hist, x, ...);                             // iteração 0

LIKWID_MARKER_START("newton");
t0 = timestamp();
for (k = 0; k < maxIt; ++k) {
    avaliaF(x, F, n);
    if (normaMax(F, n) < eps) break;

    LIKWID_MARKER_START("jacobiana");  tj = timestamp();
    jacobianaDensa(x, J, n);                        // O(n²)
    t->jacobiana += timestamp() - tj;  LIKWID_MARKER_STOP("jacobiana");

    for (i...) F[i] = -F[i];                        // laço só para trocar o sinal

    LIKWID_MARKER_START("sl");  ts = timestamp();
    erro = gaussDenso(J, F, n);                     // O(n³); Δ volta em F
    t->sl += timestamp() - ts;  LIKWID_MARKER_STOP("sl");
    if (erro) break;

    for (i...) x[i] += F[i];                        // atualiza x
    it = k + 1;
    memcpy(hist + it*n, x, ...);                    // guarda no histórico
    if (normaMax(F, n) < eps) break;                // ||Δ||
}
t->total = timestamp() - t0;
LIKWID_MARKER_STOP("newton");
```

## Decisões

- **Aloca F e J uma vez, fora do laço.** Mesmo sendo a versão não otimizada: alocar dentro
  do laço só colocaria ruído de `malloc` nas medições, sem representar o algoritmo.
- **Se o `malloc` de J falhar** (N = 20000 → 3,2 GB), mostra erro e retorna −1.
- **Δ é calculado dentro de F:** o Gauss resolve in-place, então F vira −F e depois recebe Δ.
- **Marcadores LIKWID por fora dos `timestamp()`**: START antes do t0 e STOP depois do
  cálculo do tempo. Assim o custo dos marcadores não entra no tempo medido.
- **Gauss singular:** sai do laço com mensagem. Nunca acontece neste problema.

## Passadas pelos vetores por iteração (além de F, J e SL)

1. `normaMax(F)`
2. `F = −F`
3. `x += Δ`
4. `memcpy` para o histórico
5. `normaMax(Δ)`

São 5 passadas. A versão otimizada reduz para 2 (ver 09).

## Complexidade por iteração

| Parte | Tempo | Memória |
|---|---|---|
| Jacobiana densa | O(n²) | n² doubles |
| Gauss denso | O(n³) | (in-place) |
| Resto | O(n) | — |

O sistema linear domina: **mais de 99 % do tempo a partir de N = 512** (99,96 % em N = 4000) (dados da Etapa A).
