# Notas da Etapa A (versão não otimizada) — coleta de 18/11/2026

Observações feitas na conferência dos dados de `resultados_nao-otimizado_likwid.tar.gz`.
Servem para o processamento no Windows e para o relatório. **Os dados estão válidos; não
é preciso refazer a coleta.**

## 1. Condições da coleta

| Item | Valor |
|---|---|
| Máquina | `h52`, Lab-12/DINF (acesso: `macalan.c3sl.ufpr.br` → `ssh h52`) |
| CPU | Intel Core i5-7500 @ 3.40 GHz (Kaby Lake), 4 núcleos, sem SMT |
| Caches | L1d 32 kB, L2 256 kB (por núcleo), L3 6 MB (compartilhada) |
| Núcleo | 3 (`taskset -c 3` e `likwid-perfctr -C 3`) |
| Frequência | governor `performance` via `likwid-setFrequencies -g performance -c 3`, devolvido para `powersave` ao final. **Turbo habilitado** (`no_turbo=0`), o que deve ser mencionado no relatório |
| Compilador | gcc (Debian 14.2.0-19) 14.2.0 |
| LIKWID | 5.5.2, `/home/soft/likwid` (mesma instalação usada na compilação) |
| Parâmetros | x0 = −1, ε = 0, MAX = 25 (sempre 25 iterações), 3 repetições de tempo |
| N medidos | 32, 64, 128, 256, 512, 1000, 2000, 4000 |
| Duração | ~37 min |

Outras máquinas descartadas: o notebook do Gabriel (Core Ultra 5 125H, Meteor Lake híbrido)
não é suportado pelo LIKWID, e a `macalan` é servidora compartilhada e não dá acesso aos MSR
(contadores zerados). O enunciado proíbe usar servidoras compartilhadas.

## 2. Flags de compilação

- O enunciado do trabalho pede `-O3 -march=native -mavx -fopt-info-vec`.
- **A Etapa A foi compilada com `-O3 -mavx2 -march=native -fopt-info-vec`**, por engano: uma
  página geral do LIKWID cita `-mavx2`. O `Makefile` já foi corrigido para as flags do
  enunciado (commit `5d7ab42`).
- **Isso não altera os resultados.** No i5-7500, `-march=native` já habilita AVX e AVX2, e
  compilando todos os `.c` com `-march=skylake -mavx` e com `-march=skylake -mavx2` (Kaby Lake
  tem o mesmo conjunto de instruções do Skylake), o código de máquina saiu **idêntico em todos
  os arquivos**. Basta uma frase no relatório.

## 3. Tempos médios (ms, média de 3 repetições)

| N | Total Newton | Jacobianas | Sistemas lineares | Variação máx. entre reps |
|---|---|---|---|---|
| 32 | 0,120 | 0,005 | 0,112 | 2,1 % |
| 64 | 0,670 | 0,029 | 0,632 | 12,7 % |
| 128 | 4,747 | 0,116 | 4,613 | 3,5 % |
| 256 | 35,48 | 0,493 | 34,95 | 1,0 % |
| 512 | 272,7 | 2,008 | 270,6 | 1,0 % |
| 1000 | 2 396 | 8,096 | 2 388 | 1,6 % |
| 2000 | 30 454 | 32,80 | 30 420 | 0,8 % |
| 4000 | 268 954 | 119,8 | 268 833 | 0,2 % |

- O sistema linear (Gauss denso, O(n³)) domina: mais de 99,9 % do tempo a partir de N=512.
- A Jacobiana densa é O(n²), porque zera e preenche n×n posições a cada iteração. O tempo dela
  multiplica por ~4 quando N dobra.
- Para N pequeno a variação relativa é maior (tempos de microssegundos); é normal.

## 4. N > 4000 não medidos na versão não otimizada

O enunciado pede N até 20000. Pelo O(n³), a partir dos 269 s de N=4000, cada execução levaria:

| N | 1 execução | 7 execuções (3 tempo + 4 grupos) |
|---|---|---|
| 8000 | ~36 min | ~4 h |
| 9000 | ~51 min | ~6 h |
| 10000 | ~70 min | ~8 h |
| 20000 | ~9,6 h (matriz de 3,2 GB) | ~67 h |

Justificativa para o relatório: inviável no tempo disponível. A tendência O(n³) já está clara
até 4000, e a versão otimizada cobre todos os N.

## 5. Banda de memória (MEM): a região `newton` não vale em N = 2000 e 4000

No i5-7500 (cliente), os contadores de memória do grupo MEM (uncore, lidos por MMIO) têm
**32 bits** e contam linhas de 64 B, então dão a volta a cada 2³² × 64 B ≈ **274,9 GB**.
O LIKWID só lê os contadores no START/STOP de cada região e consegue corrigir no máximo uma
volta entre as duas leituras.

| N | Volume `sl` | Volume `newton` | Diferença |
|---|---|---|---|
| 1000 | 16,32 GB | 16,44 GB | ok (newton ≥ sl) |
| 2000 | 652,2 GB | 378,1 GB | 274,1 GB = **1 volta perdida** |
| 4000 | 6 613 GB | 294,1 GB | 6 319 GB = **23 voltas perdidas** |

- A região `newton` contém a `sl`, então o volume dela nunca poderia ser menor. A diferença é
  um número inteiro de voltas do contador, o que confirma o estouro.
- **`jacobiana` e `sl` estão corretas**: cada chamada fica abaixo de 274,9 GB (a maior,
  sl em N=4000, ~264,5 GB por chamada), e os valores são somados chamada a chamada.
- **Banda de `newton` corrigida** (se quiser plotar): volume_corrigido = volume_medido +
  k × 274,878 GB, com k = 1 em N=2000 e k = 23 em N=4000, dividido pelo tempo da região
  (`RDTSC Runtime`). Isso dá 21,5 GB/s (N=2000) e 24,6 GB/s (N=4000), praticamente iguais ao `sl`. Outra opção é
  plotar só até N=1000 para `newton`, explicando o motivo.
- O FLOPS e o L2CACHE usam contadores de núcleo de 48 bits, que não têm esse problema.
- **Etapa B:** a versão otimizada é O(n) e trafega muito menos dados, então não deve estourar.
  Conferir mesmo assim (volume de `newton` ≥ volume de `sl` + `jacobiana`).

## 6. Leitura esperada dos gráficos (versão não otimizada)

Tamanho da matriz densa: 8·N² bytes. N=128 → 128 kB (cabe na L2), N=256 → 512 kB e
N=512 → 2 MB (cabem na L3), N=1000 → 8 MB (já não cabe na L3 de 6 MB), N=4000 → 128 MB.

- **FLOPS_DP / FLOPS_AVX (`sl`)**: sobe até ~8,2 GFLOP/s em N=256 (dados na cache) e cai para
  ~7,0 GFLOP/s em N=1000 e ~4,0 GFLOP/s em N=4000, quando passa a ser limitado pela memória.
  Praticamente todos os FLOPs são AVX (256 bits): o laço interno do Gauss foi vetorizado.
- **FLOPS da `jacobiana`**: perto de zero e 0 em AVX, porque ela quase só escreve (zera e
  preenche a matriz) e faz uma conta por linha.
- **MEM**: para N ≤ 512 os valores são baixos e ruidosos, porque os dados cabem na cache e o
  contador é do socket inteiro (pega qualquer atividade de fundo). A partir de N=1000 a banda
  sobe: o `sl` chega a ~24,6 GB/s em N=4000, perto do limite da DDR4 da máquina, e a
  `jacobiana` a ~26,6 GB/s (escrita de 128 MB por iteração).
- **FLOPS da região `newton` em N=32**: baixo (~0,17 GFLOP/s) por causa do custo dos próprios
  marcadores LIKWID chamados dentro dela, que pesa quando o trabalho é minúsculo.
- **L2CACHE**: usar a métrica "L2 miss ratio" (o enunciado pede "data cache miss ratio").

## 7. Conteúdo do `.tar.gz`

- `tempos.csv`: versao, N, rep, total_ms, jacobiana_ms, sl_ms
- `raw/nao-otimizado_<GRUPO>_<N>.txt`: saída do `likwid-perfctr -m`, regiões newton/jacobiana/sl
- `topologia.txt`: `likwid-topology -c -g` (seção Arquitetura)
- `sistema.txt`: máquina, governor, núcleo, versões, flags e `lscpu`
- `compilacao.log` e `compilacao_likwid.log`: saída do `-fopt-info-vec` (laços vetorizados)
- `grupos.txt`, `teste_marcadores.txt`: grupos usados e teste de sanidade
