# 14. Etapa A: coleta da versão não otimizada (18/11/2026)

Detalhes completos: `likwid/NOTAS_ETAPA_A.md`. Dados:
`resultados_coleta/resultados_nao-otimizado_likwid.tar.gz`.

## Por que em duas etapas

A versão não otimizada é a demorada (~37 min) e não depende do solver do Arthur. Dava para
coletá-la antes, enquanto ele terminava. A Etapa B foi feita depois, **na mesma máquina e nas
mesmas condições**, como exige o enunciado.

## Condições

| Item | Valor |
|---|---|
| Máquina | `h52`, Lab-12/DINF (acesso: `macalan.c3sl.ufpr.br` → `ssh h52`) |
| CPU | Intel Core i5-7500 @ 3,40 GHz (Kaby Lake; o LIKWID a chama de "Coffeelake"), 4 núcleos, sem SMT |
| Caches | L1d 32 kB (8 vias), L2 256 kB (4 vias) por núcleo; L3 6 MB (12 vias, inclusiva, compartilhada); linha de 64 B |
| Núcleo | 3 |
| Frequência | governor `performance`, **turbo ligado** (`no_turbo=0`), clock medido entre ~3,66 e 3,82 GHz (Intel HWP ativo) |
| Software | Debian, kernel 6.12.107, gcc 14.2.0, LIKWID 5.5.2 |
| Executáveis | idênticos aos gerados com as opções exigidas, `-O3 -march=native -mavx -fopt-info-vec` (ver 03) |
| Parâmetros | x0 = −1, ε = 0, MAX = 25, 3 repetições |
| N | 32, 64, 128, 256, 512, 1000, 2000, 4000 |
| Duração | ~37 min |

## Tempos médios (ms)

| N | Total | Jacobianas | SL | SL / total |
|---|---|---|---|---|
| 32 | 0,120 | 0,005 | 0,112 | 93 % |
| 64 | 0,670 | 0,029 | 0,632 | 94 % |
| 128 | 4,75 | 0,116 | 4,61 | 97 % |
| 256 | 35,5 | 0,493 | 34,9 | 98,5 % |
| 512 | 273 | 2,01 | 271 | 99,2 % |
| 1000 | 2 396 | 8,10 | 2 388 | 99,7 % |
| 2000 | 30 454 | 32,8 | 30 420 | 99,9 % |
| 4000 | 268 954 | 120 | 268 833 | 99,96 % |

- Quando N dobra, o SL multiplica por ~8 (O(n³)) e a Jacobiana por ~4 (O(n²)).

## Por que parou em N = 4000

Pela tendência O(n³) a partir dos 269 s de N = 4000:

| N | 1 execução | 7 execuções (3 de tempo + 4 grupos) |
|---|---|---|
| 8000 | ~36 min | ~4 h |
| 10000 | ~70 min | ~8 h |
| 20000 | ~9,6 h (matriz de 3,2 GB) | ~67 h |

Inviável no tempo do trabalho. A tendência já está clara até 4000, e a versão otimizada cobre
todos os N.

## Problema encontrado: estouro do contador MEM na região `newton`

| N | Volume `sl` | Volume `newton` | Diferença |
|---|---|---|---|
| 1000 | 16,32 GB | 16,44 GB | ok |
| 2000 | 652,2 GB | 378,1 GB | 274,1 GB = 1 volta perdida |
| 4000 | 6 613 GB | 294,1 GB | 23 voltas perdidas |

A região `newton` **contém** a `sl`, então seu volume nunca pode ser menor. A diferença é um
número inteiro de voltas de 274,9 GB: o contador de 32 bits estourou várias vezes dentro da
única chamada de `newton`. As regiões `jacobiana` e `sl` estão certas, porque cada chamada
delas fica abaixo de 274,9 GB e o LIKWID soma chamada a chamada.

**Correção possível:** volume_corrigido = medido + k·274,878 GB (k = 1 em N = 2000, k = 23 em
N = 4000), dividido pelo `RDTSC Runtime` da região. Dá 21,5 GB/s e 24,6 GB/s, praticamente
iguais ao `sl`. Alternativa: plotar a `newton` só até N = 1000 e explicar.
