# 10. Todas as otimizações: onde, como, por que e qual slide

É a base da **seção de Otimizações do relatório**. O enunciado pede, para cada uma: **o que é**,
**em qual função ou trecho** e **por que melhora o desempenho**.

## Tabela principal

| # | Otimização | Onde | Por que melhora | Material |
|---|---|---|---|---|
| 1 | **Estrutura de dados**: Jacobiana tridiagonal em 3 vetores (Struct of Arrays) | `jacobianaTri`, `jacobianaTriConst`, `newton.c` | Guarda só o que pode ser ≠ 0. Memória O(n²) → O(n); em N = 20000, 3,2 GB → 480 KB, cabendo na cache | Parte 9, slides 4 e 13; matrizes k-diagonais |
| 2 | **Algoritmo**: Gauss tridiagonal em vez de Gauss denso | `gaussTridiagonal` (Arthur) | Não opera sobre zeros: O(n³) → O(n). É o maior ganho | Sistemas lineares k-diagonais |
| 3 | **Inverso guardado** (multiplicar em vez de dividir) | `gaussTridiagonal` | Divisão é ~4–5× mais lenta que multiplicação; troca n−1 divisões | Parte 9 (operações caras) |
| 4 | **Tirar do laço o que não muda** (loop-invariant code motion) | `jacobianaTriConst` chamada uma vez | `sub` e `sup` são constantes; por iteração escreve 1 vetor em vez de 3 | Parte 9, slide 8 |
| 5 | **Faça menos trabalho**: −F calculado direto | `avaliaMenosF` | Elimina o laço de troca de sinal (uma leitura e uma escrita de n doubles a menos por iteração) | Parte 9, slide 10 |
| 6 | **Fatoração / eliminação de subexpressões**: x·(2x − 3) | `avaliaMenosF` | Uma multiplicação a menos por equação; o compilador não faz por causa da associatividade em ponto flutuante | Parte 9, slides 7 e 20 |
| 7 | **Fusão de laços** | `atualizaX` | 3 passadas viram 1: cada dado é lido uma vez (localidade temporal), menos controle de laço | Parte 10, slides 9–12 |
| 8 | **Laços sem desvio** (loop peeling e bordas ignoradas) | `avaliaMenosF`, `jacobianaTri`, `jacobianaTriConst` (o loop peeling já existia em `avaliaF`, da não otimizada; não é diferença entre as versões) | Sem `if` no corpo, o GCC vetoriza (AVX, 4 doubles por instrução) | Parte 9, slides 14–15 |
| 9 | **`restrict`** (sem aliasing) | `jacobianaTri`, `jacobianaTriConst`, `avaliaMenosF`, `atualizaX`, `gaussTridiagonal` | O GCC vetoriza direto, sem gerar duas versões do laço e testar em tempo de execução | Parte 9, slide 21 |
| 10 | **`static inline`** | `avaliaMenosF`, `atualizaX` | Elimina o custo de chamada de função | Parte 9, slide 22 |
| 11 | **Memória alinhada** em 32 B | `alocaVetor` | Acessos SIMD alinhados ao tamanho do registrador AVX | Parte 9, slide 17 |
| 12 | **Alocação única, fora do laço** | `newton.c` (e também na não otimizada) | Sem `malloc` repetido nem realocação | Parte 9, slide 13 |
| 13 | **Impressão fora da região medida** | `main.c` | O `printf` de 520 mil linhas não entra no tempo nem nas métricas | Metodologia de medição (Parte 8) |

O item 13 não é otimização do cálculo, mas é decisão de medição e vale citar na metodologia.

## Confirmação pelo `-fopt-info-vec` (compilação no Windows, igual à da `h52`)

| Laço | Resultado |
|---|---|
| `jacobiana.c:42` (`jacobianaTriConst`) | vetorizado, 32 bytes |
| `jacobiana.c:54` (`jacobianaTri`) | vetorizado, 32 bytes, sem "versioned for aliasing" |
| `newton.c:30` (`avaliaMenosF`) | vetorizado, 32 bytes |
| `broyden.c:11` (`avaliaF`, não otimizada) | vetorizado, mas "versioned for aliasing" (sem `restrict`) |
| `jacobiana.c:14` (zerar a matriz densa) | vetorizado, "versioned for aliasing" |
| `sistemas_lineares.c:41/50` (Gauss denso) | vetorizado |
| `sistemas_lineares.c:65/75` (Gauss tridiagonal) | **não vetoriza**: recorrência |
| `atualizaX` | **não vetoriza**: redução de máximo |

Na `h52`, a saída real está em `compilacao.log` dentro de cada `.tar.gz`.

## Parte 9 (Otimização Básica): o que não se aplica, e por quê

| Slide | Técnica | Motivo |
|---|---|---|
| 6 | Dimensão potência de 2 / cache thrashing | Só há vetores 1D na otimizada. Vale **citar** ao analisar N = 256 e 512 da versão densa |
| 9 | Evitar `sqrt` | Não há `sqrt` (a norma é a do máximo) |
| 11–12 | Tabela de lookup | Não há funções caras (`exp`, `tanh`...) |
| 13 | Tipos menores | Precisa de `double` |
| 18 | Intrínsecas SIMD | O compilador já vetoriza o que pode; intrínsecas prenderiam o código a uma máquina |
| 19 | `-Ofast` / `-ffast-math` | **Proibido**: as flags são fixas |

## Parte 10 (Otimização de Acesso a Dados)

### Classificação dos trechos (slides 8 e 13)

| Trecho | Não otimizada | Otimizada |
|---|---|---|
| Jacobiana | O(N²) dados / O(N) contas úteis | **Caso 1**: O(N)/O(N) |
| Sistema linear | O(N²) dados / O(N³) contas | **Caso 1**: O(N)/O(N) |
| Resto do Newton | O(N)/O(N) | **Caso 1**: O(N)/O(N) |

Na otimizada, tudo é "Caso 1": limitado pela memória quando os dados não cabem na cache, com
"potencial de otimização limitado". A ferramenta indicada para esse caso é a fusão de laços,
que foi a usada.

### Não se aplica

| Técnica | Motivo |
|---|---|
| Loop unrolling | `-O3` já desenrola/vetoriza; no Gauss tridiagonal desenrolar não remove a dependência |
| Unroll & jam | Exige dois laços aninhados reusando um vetor (ex.: MVM); a otimizada não tem laços aninhados |
| Loop blocking | Serve para matrizes densas que não cabem na cache; a otimizada só tem vetores (~640 KB em N = 20000) |
| Padding | Só faria sentido na matriz 2D (versão densa, que é a baseline de propósito) |

Unroll & jam e blocking **poderiam** ser aplicados ao Gauss denso, mas isso seria otimizar a
baseline. O ganho real vem de **não usar** a matriz densa.

### Code balance B_C (slide 4): bytes transferidos por flop

| Trecho | Estimativa | Leitura |
|---|---|---|
| `jacobianaTri` | lê x[i] (8 B), escreve diag[i] (8 B + 8 B de write-allocate) para 2 flops | ~8–12 B/flop → limitado pela memória quando não cabe na cache |
| Gauss tridiagonal | ~9 flops para ~6 doubles (~48 B) por linha | ~5 B/flop |
| Gauss denso | O(N²) dados para O(N³) flops | B_C cai com N → limitado pelo cálculo enquanto a matriz cabe na cache |

- **Write-allocate (slide 6):** antes de escrever, a CPU traz a linha de cache da memória.
  Escrever custa mais do que ler e ajuda a explicar a banda medida (por exemplo, a Jacobiana
  densa, que só escreve, tem banda alta de "evict").
- **Linha de cache = 64 B = 8 doubles (slide 2):** acesso sequencial gera 1 miss a cada 8
  elementos. Daí o L2 miss ratio na casa de 0,2–0,3.
