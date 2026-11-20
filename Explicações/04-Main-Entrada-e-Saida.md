# 04. `main.c`: entrada, histórico e saída

## O que faz, em ordem

1. `scanf("%d %lf %lf %d", &n, &x0, &eps, &maxIt)`.
2. Valida: 4 valores lidos, `N >= 2` (as fórmulas de f_1 e f_n usam x_2 e x_{n−1}) e
   `MAX >= 0`. Se falhar, mensagem em `stderr` e retorno 1.
3. Aloca `x` (n doubles) e `hist` ((MAX+1)·n doubles).
4. Preenche `x` com x0 em todas as posições.
5. `LIKWID_MARKER_INIT` e `LIKWID_MARKER_REGISTER` das três regiões.
6. Chama `newton(x, n, eps, maxIt, hist, &t)`.
7. `LIKWID_MARKER_CLOSE`.
8. Imprime o histórico e os tempos.

## Decisão principal: imprimir só depois do Newton

O enunciado pede todas as iterações na saída. Se o `printf` fosse feito dentro do laço do
Newton, o tempo de impressão entraria no tempo total e nas métricas do LIKWID. Em N = 20000,
são 26 × 20000 = **520 mil linhas**, muito mais lento que o próprio cálculo da versão
otimizada (~7 ms).

Solução: o `newton()` copia cada iterado para `hist`, e o `main` imprime tudo **depois**,
fora da região medida.
- Custo de memória: (MAX+1)·N doubles = 26 × 20000 × 8 B ≈ **4,2 MB** em N = 20000.
- `hist[k*n .. k*n+n-1]` = iterado k; k = 0 é o chute inicial.

## Formato da saída

```
x1 = -1                       <- iteração 0 (chute inicial)
...
xn = -1
#
x1 = ...                      <- iteração 1
...
#
...                           <- até a iteração 25
###########
0.39290000                    <- tempo total do Newton (ms)
0.00650000                    <- soma das Jacobianas (ms)
0.25380000                    <- soma dos sistemas lineares (ms)
###########
```

- x com `%.15g`: até 15 algarismos significativos, sem zeros inúteis.
- Tempos com `%.8lf`, em milissegundos.
- `#` só **entre** iterações (o `if (k > 0)` antes de cada bloco), como no modelo do enunciado.

## `LIKWID_MARKER_REGISTER`

Registra as regiões antes da primeira medição. Sem isso, o custo de criar a região (alocar
as estruturas internas do LIKWID) cairia dentro do primeiro `START`, distorcendo a medida da
primeira chamada.

## Retorno do `newton()`

Número de iterações feitas (o histórico tem `it + 1` vetores), ou −1 em erro de memória. Com
−1, o `main` libera a memória e sai com código 1 sem imprimir.
