# 09. `newton.c`: a versão otimizada

Mesma interface e **mesmos critérios de parada** da versão não otimizada (de propósito: só
muda o que foi otimizado). O que muda:

| | `newton_nao_otimizado.c` | `newton.c` |
|---|---|---|
| Jacobiana guardada em | `J`, n·n doubles | `sub`, `diag`, `sup`, 3·n doubles |
| Cálculo da Jacobiana | `jacobianaDensa`, tudo, toda iteração, O(n²) | `jacobianaTriConst` uma vez + `jacobianaTri` (só a diagonal), O(n) |
| Sistema linear | `gaussDenso`, O(n³) | `gaussTridiagonal`, O(n) |
| F | `avaliaF` + laço de troca de sinal | `avaliaMenosF` (calcula −F direto) |
| Atualização de x | 3 laços (soma, `memcpy`, `normaMax`) | 1 laço (`atualizaX`) |
| Passadas extras pelos vetores por iteração | 5 | 2 |
| Alocação | `malloc` | `alocaVetor` (alinhada em 32 B) |
| Memória em N = 20000 | 3,2 GB | ~640 KB (4 vetores) |

## Fluxo

```c
F, sub, diag, sup = alocaVetor(n)              // uma vez, alinhados
memcpy(hist, x, ...)                           // iteração 0

START("newton"); t0 = timestamp();

START("jacobiana"); tj = timestamp();
jacobianaTriConst(sub, sup, n);                // parte constante, uma vez
t->jacobiana += ...; STOP("jacobiana");

for (k = 0; k < maxIt; ++k) {
    avaliaMenosF(x, F, n);                     // F recebe -F(x)
    if (normaMax(F, n) < eps) break;           // ||-F|| = ||F||

    START("jacobiana"); jacobianaTri(x, diag, n); STOP("jacobiana");
    START("sl");        gaussTridiagonal(sub, diag, sup, F, n); STOP("sl");   // Δ em F

    it = k + 1;
    normaDelta = atualizaX(x, F, hist + it*n, n);   // x += Δ, histórico e ||Δ||
    if (normaDelta < eps) break;
}
t->total = timestamp() - t0; STOP("newton");
```

## `avaliaMenosF`: −F calculado direto

```c
menosF[0] = x[0]*(2.0*x[0] - 3.0) + 2.0*x[1] - 1.0;
for (int i = 1; i < n - 1; ++i)
    menosF[i] = x[i]*(2.0*x[i] - 3.0) + x[i-1] + 2.0*x[i+1] - 1.0;
menosF[n-1] = x[n-1]*(2.0*x[n-1] - 3.0) + x[n-2];
```

- **Faça menos trabalho** (slide 10 da Parte 9): o sistema linear precisa de −F. Calculando
  −F direto, o laço de troca de sinal deixa de existir: uma leitura e uma escrita a menos de
  um vetor de n posições por iteração.
- **Fatoração / eliminação de subexpressões** (slide 7): −(−2x² + 3x) = 2x² − 3x =
  **x·(2x − 3)**, uma multiplicação a menos por equação. O compilador **não** faz isso sozinho,
  porque em ponto flutuante a associatividade não vale (slide 20). Por isso o resultado muda
  na última casa decimal (~1e−15).
- **`restrict`**: vetoriza direto, sem "versioned for aliasing".
- **Loop peeling** (sem `if` no laço), como em `avaliaF`.

## `atualizaX`: fusão de laços

```c
double max = 0.0;
for (int i = 0; i < n; ++i) {
    double a = fabs(delta[i]);
    max = (a > max) ? a : max;     // ||Δ||∞
    x[i] += delta[i];              // x = x + Δ
    histK[i] = x[i];               // copia para o histórico
}
return max;
```

Três laços (soma, `memcpy`, `normaMax(Δ)`) viram **um** (slides 9–12 da Parte 10).

**Por que melhora:** cada `delta[i]` é carregado **uma vez** e usado para a norma e para a
soma enquanto está no registrador; `x[i]` acabou de ser calculado e já vai para o histórico.
Em N grande, com laços separados o vetor pode ter saído da cache antes do laço seguinte
lê-lo. Também há menos controle de laço (incremento, comparação, desvio).

**Contrapartida (bom ponto para o relatório):** o laço separado `x += Δ` vetorizava; o
fundido **não vetoriza**, por causa da redução de máximo (precisaria de `-ffast-math`,
proibido). Troca-se vetorização por menos passadas na memória. Pela Parte 10 (slide 8), laços
O(N)/O(N) como este são limitados pela memória, então reduzir passadas é o que importa.

**Por que não fundir F com a norma?** A redução de máximo impediria a vetorização do laço de
F, que hoje vetoriza.

**Por que não fundir F com a Jacobiana** (as duas leem `x[i]`)? O enunciado exige medir a
Jacobiana **separadamente**. Fundir misturaria os trechos.

## `static inline`

`avaliaMenosF` e `atualizaX` são `static inline`: o corpo é copiado no ponto de chamada, sem
custo de chamada de função. Slide 22 da Parte 9: "não confiar no compilador para fazer
inlining; usar a palavra-chave `inline`". O ganho aqui é pequeno, porque cada função é
chamada uma vez por iteração e não dentro de um laço por elemento.

## `alocaVetor` / `liberaVetor`: memória alinhada

```c
size_t bytes = ((size_t) n * sizeof(double) + 31) / 32 * 32;   // múltiplo de 32
return aligned_alloc(32, bytes);          // Windows: _aligned_malloc (só para testes)
```

Alinhado em 32 bytes, o tamanho de um registrador AVX (4 doubles), para os acessos SIMD
serem alinhados (slide 17 da Parte 9). `aligned_alloc` exige tamanho múltiplo do
alinhamento, daí o arredondamento. O `#ifdef _WIN32` existe só para testar no Windows; no
Linux, onde as medições valem, é `aligned_alloc`.

## O que fica fora das regiões `jacobiana` e `sl`

A região `newton` inclui também `avaliaMenosF`, as normas e `atualizaX`. Na versão otimizada
isso é **~1/3 do tempo total** (N = 20000: 6,6 ms no total contra 4,5 ms de `sl` +
`jacobiana`). Na não otimizada era invisível, porque o Gauss O(n³) dominava tudo.
