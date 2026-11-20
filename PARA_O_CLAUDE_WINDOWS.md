# Para o Claude no Windows: o que foi feito no Linux (18 e 19/11/2026)

Leia este arquivo primeiro. Ele resume as duas sessões de coleta com o LIKWID feitas no Linux
e diz onde está cada coisa. **A coleta terminou e não deve ser refeita.** O que falta é com
você: os 12 gráficos e o relatório.

## 1. Situação

| Item | Estado |
|---|---|
| Etapa A: versão não otimizada (`broyden-nao-otimizado`) | ✅ Coletada em 18/11, N = 32 a 4000 |
| Etapa B: versão otimizada (`broyden`, solver do Arthur) | ✅ Coletada em 19/11, N = 32 a 20000 |
| Solver tridiagonal (`gaussTridiagonal`) | ✅ Conferido: mesma saída da versão não otimizada (diferença máx. 9,99e-16), sem erros no AddressSanitizer |
| Gráficos (12) | ⏳ A fazer |
| Relatório (formato de artigo) | ⏳ A fazer |

## 2. Onde está cada coisa

| Arquivo | Conteúdo |
|---|---|
| `resultados_coleta/resultados_nao-otimizado_likwid.tar.gz` | Dados da Etapa A |
| `resultados_coleta/resultados_otimizado_likwid.tar.gz` | Dados da Etapa B |
| `likwid/NOTAS_ETAPA_A.md` | **Leitura obrigatória.** Condições, tempos médios, N > 4000 não medidos, estouro do contador MEM, leitura esperada dos gráficos |
| `likwid/NOTAS_ETAPA_B.md` | **Leitura obrigatória.** Conferências da Etapa B, tempos médios, leitura esperada dos gráficos |
| `likwid/roda_testes.sh` | Script de coleta (só referência; não precisa rodar de novo) |
| `likwid/PARA_O_CLAUDE.md`, `likwid/ETAPA_B_PARA_O_CLAUDE.md`, `likwid/LEIAME_LINUX.md` | Instruções da coleta no Linux. **Já cumpridas**, servem só de histórico |

Para extrair (PowerShell, na raiz do projeto):
```
tar xzf resultados_coleta\resultados_nao-otimizado_likwid.tar.gz
tar xzf resultados_coleta\resultados_otimizado_likwid.tar.gz
```
Isso cria `resultados_nao-otimizado/` e `resultados_otimizado/`. O `.gitignore` ignora essas
pastas; não comite as pastas extraídas.

Cada pasta tem:
- `tempos.csv`: `versao,N,rep,total_ms,jacobiana_ms,sl_ms` (3 repetições por N, tempos em ms
  de `timestamp()`, medidos no binário **sem** LIKWID);
- `raw/<versao>_<GRUPO>_<N>.txt`: saída do `likwid-perfctr -C 3 -g <GRUPO> -m` (binário com
  marcadores), com as regiões `newton`, `jacobiana` e `sl`;
- `topologia.txt`: `likwid-topology -c -g` (para a seção Arquitetura do relatório);
- `sistema.txt`: máquina, kernel, gcc, flags, governor, turbo, núcleo, versão do LIKWID, `lscpu`;
- `compilacao.log` e `compilacao_likwid.log`: saída do `-fopt-info-vec` (laços vetorizados,
  útil para justificar as otimizações).

## 3. Condições das medições (para a seção de metodologia)

- **Máquina:** `h52` do Lab-12/DINF, Intel Core i5-7500 @ 3.40 GHz (Kaby Lake), 4 núcleos sem
  SMT, L1d 32 kB, L2 256 kB por núcleo, L3 6 MB compartilhada. Acesso remoto via
  `macalan.c3sl.ufpr.br`. As duas etapas foram feitas na mesma máquina.
- **Núcleo 3** (`taskset -c 3` e `likwid-perfctr -C 3`), como pede a disciplina.
- **Frequência:** governor `performance` via `likwid-setFrequencies -g performance -c 3`,
  devolvido para `powersave` ao final. **O turbo estava habilitado** (`no_turbo=0`); as tabelas
  do LIKWID mostram clock de ~3,7–3,8 GHz. Mencionar no relatório.
- **Software:** Debian, kernel 6.12.107, gcc 14.2.0, LIKWID 5.5.2 (`/home/soft/likwid`).
- **Flags:** as do enunciado, `-O3 -march=native -mavx -fopt-info-vec`. A Etapa A foi compilada
  por engano com `-mavx2`; no i5-7500 o código de máquina é **idêntico** (o `-march=native` já
  liga o AVX2; conferido arquivo por arquivo). Basta uma frase no relatório.
- **Parâmetros:** x0 = −1 (o enunciado não fixa o x0; com 0,5 o Newton não converge),
  ε = 0 e MAX = 25 (sempre 25 iterações), como exige o enunciado.
- **Máquinas descartadas:** o notebook do Gabriel (Core Ultra 5 125H, Meteor Lake híbrido) não
  é suportado pelo LIKWID, e a `macalan` é servidora compartilhada (proibida pelo enunciado)
  e não dá acesso aos contadores.

## 4. Os 12 gráficos (do enunciado)

3 trechos (`newton` = execução total, `jacobiana`, `sl`) × 4 testes. Em cada gráfico, uma linha
por versão (cores distintas), eixo X = N em **escala log**.

| Teste | Fonte | O que plotar |
|---|---|---|
| Tempo | `tempos.csv` | Tempo médio em ms, eixo Y **também em escala log**. Colunas `total_ms` (newton), `jacobiana_ms`, `sl_ms` |
| Banda de memória | `raw/*_MEM_<N>.txt` | `Memory bandwidth [MBytes/s]` |
| Cache miss L2 | `raw/*_L2CACHE_<N>.txt` | `L2 miss ratio` (é o "data cache miss ratio" do enunciado) |
| Operações aritméticas | `raw/*_FLOPS_DP_<N>.txt` e `raw/*_FLOPS_AVX_<N>.txt` | `DP [MFLOP/s]` (FLOPS_DP) e `Packed DP [MFLOP/s]` (FLOPS_AVX). O enunciado pede os dois grupos; dá para plotar as duas métricas no mesmo gráfico ou mostrar `AVX DP [MFLOP/s]`, que também aparece no FLOPS_DP |

Sobre o "tempo médio": a média das 3 repetições do valor somado que o programa imprime
(25 iterações). Também é possível dividir por 25 para ter o tempo por iteração; escolha uma
forma e diga qual no relatório.

**Formato dos `raw`:** para cada região há um bloco que começa com
`Region <nome>, Group 1: <GRUPO>`, seguido de uma tabela de eventos e de uma tabela de
métricas no formato `| <Métrica> | <valor> |`, na coluna `HWThread 3`. Como foi medido um só
núcleo, não há tabelas `STAT`. Para extrair, leia linha a linha, guarde a região atual ao ver
`Region ...` e pegue o valor da métrica dentro daquele bloco.

**Faixa de N:** a versão não otimizada só tem N até 4000. Acima disso o Gauss denso O(n³)
levaria de horas a dias (estimativas em `NOTAS_ETAPA_A.md`, seção 4). Nos gráficos, a linha
dela simplesmente termina em 4000; justifique no relatório.

## 5. Cuidados que afetam os gráficos (detalhes nas notas)

1. **Banda de memória da região `newton` da versão não otimizada em N=2000 e N=4000 está
   errada** por estouro do contador de 32 bits do MEM (a cada ~274,9 GB). Não plote os valores
   crus. Ou corrija (volume medido + k × 274,878 GB, com k = 1 em N=2000 e k = 23 em N=4000,
   dividido pelo `RDTSC Runtime` da região, o que dá 21,5 e 24,6 GB/s), ou pare a linha em
   N=1000 e explique. As regiões `jacobiana` e `sl` estão corretas. A versão otimizada não tem
   estouro. Ver `NOTAS_ETAPA_A.md`, seção 5.
2. **Banda de memória da versão otimizada é baixa e ruidosa em todos os N**: os dados cabem na
   L3 (≤ 4,3 MB de tráfego), e o contador é do socket inteiro. É esperado e é o contraste com a
   não otimizada, que chega a ~24,6 GB/s.
3. **`sl` da versão otimizada tem AVX = 0**: o Gauss tridiagonal tem dependência entre
   iterações e não vetoriza. Não é erro.
4. **MFLOP/s da região `newton` em N pequeno** é baixo nas duas versões: o custo dos marcadores
   LIKWID dentro da região domina quando o trabalho é de microssegundos.
5. **Variação entre repetições em N pequeno** (até 52 % em N=128 na otimizada, por uma
   repetição lenta). Considerar a mediana; dizer no relatório qual estatística foi usada.

## 6. Resultados principais (para conferir os gráficos)

| N | Não otimizada: total (ms) | Otimizada: total (ms) |
|---|---|---|
| 32 | 0,120 | 0,0108 |
| 256 | 35,48 | 0,0871 |
| 1000 | 2 396 | 0,393 |
| 4000 | 268 954 | 1,539 |
| 20000 | — | 6,635 |

- Em N=4000 a otimizada é ~175 000× mais rápida. No gráfico de tempo log-log, a não otimizada
  tem inclinação ~3 (O(n³)) e a otimizada ~1 (O(n)).
- Não otimizada: o `sl` passa de 99,9 % do tempo a partir de N=512; FLOPS do `sl` sobe até
  ~8,2 GFLOP/s em N=256 (matriz na cache) e cai para ~4,0 GFLOP/s em N=4000 (limitado pela
  memória), quase tudo AVX.
- Otimizada: Jacobiana ~3,5 GFLOP/s (vetorizada), `sl` ~1,0 GFLOP/s escalar.

## 7. O que não fazer

- Não refazer a coleta nem rodar no Windows para comparar: as medições valem só na `h52`.
- Não alterar código, `Makefile`, flags ou parâmetros: os dados entregues correspondem a este
  código (commit `36d095a` para a versão otimizada).
- Não comitar as pastas extraídas `resultados_*/`.
