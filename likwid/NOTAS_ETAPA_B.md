# Notas da Etapa B (versão otimizada) — coleta de 19/11/2026

Conferência dos dados de `resultados_coleta/resultados_otimizado_likwid.tar.gz`.
**Todas as conferências passaram; os dados estão válidos.**

## 1. Coleta

| Item | Valor |
|---|---|
| Data | 19/11/2026, 20:05 (a coleta completa levou poucos minutos) |
| Máquina | `h52`, Lab-12/DINF (Intel i5-7500), a mesma da Etapa A |
| Código | commit `36d095a` (solver tridiagonal do Arthur, `5ebccaf`) |
| Núcleo / frequência | CORE 3, governor `performance` via `likwid-setFrequencies`, turbo habilitado (`no_turbo=0`), igual à Etapa A |
| Flags | `-O3 -march=native -mavx -fopt-info-vec` (as do enunciado) |
| N medidos | 32, 64, 128, 256, 512, 1000, 2000, 4000, 8000, 9000, 10000, 20000 (todos) |
| Parâmetros | x0 = −1, ε = 0, MAX = 25, 3 repetições de tempo |

Antes da coleta, o teste rápido comparou as duas versões na própria `h52`: maior diferença
de 9,99e-16 em N=32 (só arredondamento). No notebook, a comparação deu a mesma diferença máxima
para N = 2, 3, 32, 100, 1000 e 2000, e o solver passou sem erros no AddressSanitizer.

## 2. Conferências

| Conferência | Resultado |
|---|---|
| Mesmas condições da Etapa A (`sistema.txt`, linhas 2–15) | ✅ Idênticas, exceto a linha `Flags`: a Etapa A usou `-mavx2` (código de máquina idêntico no i5-7500, ver `NOTAS_ETAPA_A.md`, seção 2). Mesmo kernel, gcc 14.2.0, LIKWID 5.5.2, governor, núcleo e parâmetros |
| `topologia.txt` | ✅ Idêntica à da Etapa A, exceto "Free memory" (2097 MB → 1985 MB), que só registra o momento da coleta |
| `tempos.csv` | ✅ 36 linhas (12 N × 3 repetições), nenhum campo vazio |
| `raw/` | ✅ 48 arquivos (12 N × 4 grupos), todos com as regiões `newton`, `jacobiana` e `sl`, sem erros de MSR |
| Estouro do contador MEM | ✅ Nenhum: em todos os N, o volume de `newton` ≥ `sl` + `jacobiana` (no máximo 4,3 MB em N=20000) |
| Compilação | ✅ Sem erros ou warnings |

## 3. Tempos médios (ms, média de 3 repetições)

| N | Total Newton | Jacobianas | Sistemas lineares | Variação máx. entre reps |
|---|---|---|---|---|
| 32 | 0,0108 | 0,0008 | 0,0070 | 3,1 % |
| 64 | 0,0216 | 0,0009 | 0,0136 | 3,4 % |
| 128 | 0,0508 | 0,0054 | 0,0312 | 52,2 % (*) |
| 256 | 0,0871 | 0,0016 | 0,0527 | 15,5 % |
| 512 | 0,2166 | 0,0037 | 0,1419 | 18,3 % |
| 1000 | 0,3929 | 0,0065 | 0,2538 | 14,5 % |
| 2000 | 0,8418 | 0,0177 | 0,5682 | 8,7 % |
| 4000 | 1,539 | 0,0486 | 0,9866 | 5,0 % |
| 8000 | 2,886 | 0,0983 | 1,800 | 2,3 % |
| 9000 | 3,251 | 0,1154 | 2,028 | 0,4 % |
| 10000 | 3,553 | 0,1305 | 2,202 | 4,1 % |
| 20000 | 6,635 | 0,2982 | 4,233 | 3,6 % |

(*) Em N=128, a 3ª repetição foi mais lenta (0,068 ms contra ~0,042 ms nas outras duas; a
Jacobiana levou 0,0139 ms contra ~0,0011 ms). Com tempos de dezenas de microssegundos, uma única
interrupção do sistema já causa isso. Opções para o gráfico: usar a média e comentar, ou usar a
mediana/mínimo das 3 repetições (dizer no relatório qual foi usada).

Comparação com a Etapa A em N=4000: total de 268 954 ms → 1,54 ms (~175 000×). Sistemas
lineares 268 833 ms → 0,99 ms; Jacobianas 119,8 ms → 0,049 ms.

Parte do tempo total do Newton fica fora das regiões `jacobiana` e `sl`: a avaliação de −F,
a norma e a atualização de x (por exemplo, em N=20000: 6,6 ms no total contra 4,5 ms de `sl` +
`jacobiana`).

## 4. Leitura esperada dos gráficos (versão otimizada)

Tudo é O(n) e cabe na cache: cada vetor tem 8·N bytes (160 kB em N=20000). Os 4 vetores do
Newton somam ~640 kB, e o histórico das 26 iterações (`hist`) soma ~4,2 MB em N=20000, abaixo
da L3 de 6 MB.

- **MEM**: o volume de dados que vai para a memória principal é desprezível (≤ 4,3 MB), e as
  bandas são baixas e ruidosas em todos os N. Isso é esperado: os dados ficam na cache, e o
  contador é do socket inteiro (pega atividade de fundo). É o contraste com a Etapa A, em que
  o `sl` chegava a ~24,6 GB/s.
- **FLOPS_DP / FLOPS_AVX**:
  - `jacobiana` (só `diag[i] = -4x[i] + 3`): vetorizada, ~3,5 GFLOP/s em N=20000, quase tudo AVX.
  - `sl` (Gauss tridiagonal, algoritmo de Thomas): ~1,0 GFLOP/s e **AVX = 0**. Cada linha
    depende da anterior (dependência entre iterações), então o laço não vetoriza. O
    `compilacao.log` mostra só uma vetorização parcial de bloco básico em
    `sistemas_lineares.c:67`. Isso não é erro.
  - `newton` em N pequeno: MFLOP/s muito baixo (5 MFLOP/s em N=32) por causa do custo dos
    marcadores LIKWID dentro da região, que domina quando o trabalho é de microssegundos.
- **L2CACHE**: "L2 miss ratio" entre ~0,19 e ~0,31 nas três regiões.
- **Tempo**: cresce linearmente com N (O(n)), contra O(n³) na Etapa A. No gráfico log-log, a
  inclinação é ~1 na otimizada e ~3 na não otimizada.

## 5. Observações

- Nada diferente do esperado. Sem estouro do MEM, sem mudança de software ou de máquina, e a
  `h52` estava livre.
- A saída do teste rápido colada no chat terminou no `PRONTO!`, sem a linha "Frequencia do
  nucleo 3 devolvida para 'powersave'" que costuma vir depois. É provável que só tenha ficado de
  fora na cópia. Não afeta os dados: o governor estava em `performance` durante a coleta
  (registrado no `sistema.txt`).
- Lembrar no relatório: a versão não otimizada só foi medida até N=4000 (justificativa em
  `NOTAS_ETAPA_A.md`, seção 4); a otimizada cobre todos os N.
