# 06. `jacobiana.c`: Jacobiana densa e tridiagonal

## Versão não otimizada: `jacobianaDensa(x, J, n)`

```c
for (int i = 0; i < n; ++i)              // zera a matriz inteira: n² escritas
    for (int j = 0; j < n; ++j)
        J[i*n + j] = 0.0;

for (int i = 0; i < n; ++i) {            // preenche as três diagonais
    J[i*n + i] = -4.0*x[i] + 3.0;
    if (i > 0)     J[i*n + i - 1] = -1.0;
    if (i < n - 1) J[i*n + i + 1] = -2.0;
}
```

- **Matriz em um vetor, por linhas (row-major):** elemento (i, j) em `J[i*n + j]`. Um bloco
  contíguo é melhor para a cache e mais simples que `double **` (slide 4 da Parte 9).
- **Zera tudo a cada iteração:** o `malloc` devolve lixo e quase todos os elementos são zero.
  Isso a torna **O(n²)** em tempo e escrita: n² escritas para 2n flops úteis.
- Memória: 8·N² bytes. N = 1000 → 8 MB; N = 4000 → 128 MB; N = 20000 → 3,2 GB.

## Versão otimizada: 3 vetores

A Jacobiana é tridiagonal (ver 02). Em vez da matriz n×n, guardamos **três vetores de tamanho
n**, um por diagonal: o armazenamento de matriz k-diagonal visto na matéria (Struct of
Arrays, slide 4 da Parte 9).

```
sub[i]  = ∂f_i/∂x_{i-1} = -1        (sub[0] não faz parte da matriz)
diag[i] = ∂f_i/∂x_i     = -4x_i + 3
sup[i]  = ∂f_i/∂x_{i+1} = -2        (sup[n-1] não faz parte da matriz)
```

### `jacobianaTriConst(sub, sup, n)`: uma vez só

```c
for (int i = 0; i < n; ++i) {
    sub[i] = -1.0;
    sup[i] = -2.0;
}
```

### `jacobianaTri(x, diag, n)`: a cada iteração

```c
for (int i = 0; i < n; ++i)
    diag[i] = -4.0 * x[i] + 3.0;
```

### Otimizações aplicadas aqui

1. **Só as 3 diagonais.** O(n) em vez de O(n²), em tempo e memória. Em N = 20000 são 480 KB
   em vez de 3,2 GB: cabe na cache.
2. **Tirar do laço o que não muda (loop-invariant code motion).** `sub` e `sup` não dependem
   de x: são preenchidas **uma vez**, antes do laço do Newton, e cada iteração só reescreve a
   diagonal. Por iteração, 1 vetor escrito em vez de 3. Slide 8 da Parte 9. Só funciona
   porque o `gaussTridiagonal` recebe `sub` e `sup` como `const` e não os altera.
3. **Laço sem `if`.** `sub[0]` e `sup[n−1]` são preenchidos mesmo não fazendo parte da
   matriz; o Gauss tridiagonal nunca os lê. Sem testar as bordas, o laço vetoriza.
4. **`restrict`.** O GCC vetoriza direto, sem a versão alternativa "versioned for aliasing".
   `-fopt-info-vec`: `jacobiana.c:42` e `:54`, "loop vectorized using 32 byte vectors" (AVX).

### Tempo da `jacobianaTriConst` entra na conta

A chamada única fica dentro da região `jacobiana` e do tempo `t->jacobiana`. Assim a
comparação é honesta: o custo dela não some.

Efeito colateral visível nos dados: na versão otimizada, o LIKWID mostra **call count = 26**
na região `jacobiana` (1 da parte constante + 25 da diagonal). Na não otimizada são 25.
Lembre disso se for calcular "tempo por chamada".
