# 12. LIKWID: instrumentação, grupos e métricas

## O que é

LIKWID (*Like I Knew What I'm Doing*) lê os **contadores de hardware** do processador
(instruções, ciclos, operações de ponto flutuante, linhas de cache trocadas com a memória) e
calcula métricas como MFLOP/s e banda de memória. Só funciona em Linux nativo, com acesso aos
registradores MSR, e só em processadores que ele suporta.

## Marker API: medir só um trecho do programa

```c
LIKWID_MARKER_INIT;                     // main, uma vez
LIKWID_MARKER_REGISTER("newton");       // main, antes de medir
...
LIKWID_MARKER_START("jacobiana");
jacobianaTri(x, diag, n);
LIKWID_MARKER_STOP("jacobiana");
...
LIKWID_MARKER_CLOSE;                    // main, no fim
```

Regiões criadas (uma por trecho do enunciado):

| Região | Envolve | Chamadas |
|---|---|---|
| `newton` | o método inteiro | 1 |
| `jacobiana` | cada cálculo de Jacobiana | 25 (não otimizada) / 26 (otimizada: +1 da parte constante) |
| `sl` | cada resolução de sistema linear | 25 |

Regiões chamadas várias vezes **acumulam**: o LIKWID soma os contadores de todas as chamadas.

## `likwid_markers.h`

```c
#ifdef LIKWID_PERFMON
#include <likwid.h>           // Linux com make LIKWID=1: marcadores reais
#else
#define LIKWID_MARKER_START(t)  // em qualquer outro caso: macros vazias
...
#endif
```

O mesmo código compila no Windows (sem LIKWID) e no Linux. O nome tem de ser **exatamente**
`LIKWID_PERFMON`: com outro nome, os marcadores viram nada sem nenhum erro (esse bug apareceu
no começo e foi corrigido).

## Como roda

```bash
make LIKWID=1
echo "1000 -1 0 25" | likwid-perfctr -C 3 -g FLOPS_DP -m ./broyden
```

- `-C 3`: prende o programa no núcleo 3 e mede esse núcleo.
- `-g GRUPO`: qual conjunto de contadores ler.
- `-m`: usa os marcadores (sem `-m`, mede o programa inteiro).

## Grupos e métricas usados

| Grupo | Contadores (resumo) | Métrica do gráfico | Significado |
|---|---|---|---|
| `MEM` | linhas de 64 B lidas/escritas no controlador de memória (uncore, do socket inteiro) | `Memory bandwidth [MBytes/s]` | quanto dado trafegou entre RAM e processador por segundo |
| `L2CACHE` | acessos (`L2_RQSTS_ALL_DEMAND_REFERENCES`) e misses da L2 | `L2 miss ratio` | fração dos acessos à L2 que não acharam o dado (o "data cache miss ratio" do enunciado) |
| `FLOPS_DP` | instruções de ponto flutuante double: escalar, 128 bits e 256 bits | `DP [MFLOP/s]` (também mostra `AVX DP [MFLOP/s]`) | milhões de operações double por segundo, todas as larguras |
| `FLOPS_AVX` | só instruções AVX de 256 bits | `Packed DP [MFLOP/s]` | milhões de operações double por segundo **só com AVX** |

Outras métricas úteis nos arquivos:
- `Runtime (RDTSC) [s]`: tempo da região;
- `Clock [MHz]`: ~3780 MHz (turbo ligado);
- `CPI`: ciclos por instrução;
- `Memory data volume [GBytes]`: volume total de dados da região.

### Contas do FLOPS

- Uma instrução AVX de 256 bits em double = **4 flops**; SSE 128 bits = 2; escalar = 1.
- `DP MFLOP/s` = (escalar + 2·SSE + 4·AVX) / tempo.
- `Packed DP MFLOP/s` (FLOPS_AVX) = 4·AVX / tempo.
- Se FLOPS_AVX ≈ FLOPS_DP, quase todas as contas foram vetorizadas com AVX. Se FLOPS_AVX = 0,
  o código é escalar.

## O que é DP e o que é AVX (os dois grupos de FLOPS)

São duas formas de contar as contas de ponto flutuante do programa. A diferença está em
**quantas contas cada instrução faz**.

### DP = *Double Precision* (precisão dupla)

São as contas com `double`, o tipo usado em todo o código (x, F, Jacobiana). O grupo
**FLOPS_DP** conta **todas** as operações com double, qualquer que seja a instrução usada:

| Tipo de instrução | Contas por instrução |
|---|---|
| Escalar (normal) | 1 |
| SSE (128 bits) | 2 doubles de uma vez |
| AVX (256 bits) | 4 doubles de uma vez |

Métrica do gráfico: **`DP [MFLOP/s]`**, o total de contas com double por segundo, em milhões.

### AVX = *Advanced Vector Extensions*

São instruções **vetoriais** (SIMD: *Single Instruction, Multiple Data*). O processador tem
registradores de 256 bits, onde cabem **4 doubles**, e **uma instrução faz a mesma conta nos 4
ao mesmo tempo**:

```
sem AVX (escalar):              com AVX (vetorizado):
diag[0] = -4*x[0] + 3           diag[0..3] = -4*x[0..3] + 3   <- 1 instrução, 4 contas
diag[1] = -4*x[1] + 3
diag[2] = -4*x[2] + 3
diag[3] = -4*x[3] + 3           <- 4 instruções
```

O grupo **FLOPS_AVX** conta **só** as contas feitas com essas instruções de 256 bits.
Métrica do gráfico: **`Packed DP [MFLOP/s]`** ("packed" = empacotado, 4 doubles juntos).

### Como ler os dois juntos

| Situação | Significado | Exemplo no trabalho |
|---|---|---|
| **AVX ≈ DP** | quase todas as contas foram vetorizadas | `sl` não otimizado: o laço interno do Gauss denso vetoriza |
| **AVX = 0** | código totalmente escalar | `sl` otimizado: o Gauss tridiagonal não vetoriza (cada linha depende da anterior); `jacobiana` não otimizada: as contas úteis ficam no laço com `if` |
| **AVX alto** | o laço vetorizou | `jacobiana` otimizada: `diag[i] = -4x[i]+3`, sem `if` e com `restrict` |

**Quem vetoriza é o compilador**, por causa de `-O3 -march=native -mavx`. O `-fopt-info-vec`
avisa na compilação quais laços ele vetorizou ("loop vectorized using 32 byte vectors": 32
bytes = 256 bits = 4 doubles; ver `compilacao.log` dentro dos `.tar.gz`). O FLOPS_AVX
**confirma com o programa rodando** o que o compilador prometeu, e essas são duas evidências
independentes para o relatório.

**Cuidado:** vetorizar acelera a conta em até 4×, mas **não muda a complexidade**. O Gauss
denso vetorizado (AVX alto) continua O(n³) e perde de longe para o tridiagonal escalar
(AVX = 0), que é O(n).

## Cuidados descobertos na coleta

1. **Estouro do contador MEM.** No i5-7500, os contadores do MEM têm 32 bits e contam linhas
   de 64 B: dão a volta a cada 2³²·64 B ≈ **274,9 GB**. O LIKWID só lê no START/STOP, então uma
   região que move mais que isso numa chamada perde voltas. Aconteceu na região `newton` da
   versão não otimizada em N = 2000 e 4000 (ver 14).
2. **MEM é do socket inteiro.** Conta o tráfego de todos os núcleos (atividade de fundo
   também). Com regiões de microssegundos e pouco tráfego, isso gera valores absurdos (ver 18).
3. **Custo dos marcadores.** Cada START/STOP lê contadores (alguns microssegundos). Em N
   pequeno, isso pesa nas métricas da região `newton`, que contém 51 pares de START/STOP
   internos.

## Máquinas

| Máquina | Resultado |
|---|---|
| Notebook do Gabriel (Core Ultra 5 125H, Meteor Lake) | ❌ CPU não suportada pelo LIKWID |
| `macalan` (servidora do DINF) | ❌ Compartilhada (proibida) e não dá acesso aos MSR (contadores zerados) |
| **`h52`, Lab-12/DINF** (i5-7500) | ✅ Usada nas duas etapas |
