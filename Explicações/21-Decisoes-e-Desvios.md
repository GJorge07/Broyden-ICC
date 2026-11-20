# 21. Decisões tomadas e desvios do enunciado

## Decisões de projeto

| Decisão | Motivo | Ver |
|---|---|---|
| `main()` sozinha em `main.c` | exigência do enunciado | 04 |
| Mesma função `newton()` em dois arquivos | comparação justa entre versões | 03 |
| Impressão só depois do Newton | `printf` não contamina tempo nem métricas | 04 |
| Matriz densa em um vetor row-major | memória contígua, melhor para a cache | 06 |
| Pivoteamento parcial no Gauss denso | estabilidade (versão de referência "correta") | 07 |
| Jacobiana otimizada em 3 vetores | só guarda o que pode ser ≠ 0 | 06 |
| Gauss tridiagonal sem pivoteamento | mantém a estrutura; a matriz é diagonal dominante | 07 |
| `jacobianaTriConst` uma vez, contada no tempo da Jacobiana | tirar do laço o que não muda, sem esconder o custo | 06 |
| `avaliaMenosF`, `atualizaX`, `inline`, `restrict`, alinhamento | ver tabela de otimizações | 10 |
| x0 = −1 | x0 = 0,5 não converge | 02 |
| `utils.c` próprio, `CLOCK_MONOTONIC_RAW` em ms | não há `utils` do professor; é o padrão da disciplina | 11 |
| Tempo medido sem marcadores; métricas com marcadores | instrumentação não interfere no tempo | 11, 13 |
| Coleta em duas etapas na mesma máquina | não esperar o solver; mesmas condições | 14, 15 |
| Núcleo 3, governor `performance` | regra do laboratório e estabilidade da frequência | 13 |
| 3 repetições de tempo, 1 por grupo do LIKWID | ruído do tempo × duração da coleta | 13 |

## Desvios do enunciado e como justificar

### 1. Versão não otimizada medida só até N = 4000

- **Motivo:** O(n³). N = 20000 levaria ~9,6 h por execução, ~67 h no total, e precisaria de uma
  matriz de 3,2 GB.
- **No relatório:** citar as estimativas (14) e dizer que a tendência já está clara até 4000 e
  que a otimizada cobre todos os N.

### 3. Turbo ligado

- O enunciado não exige desligar. O governor ficou em `performance`; como o Intel HWP está
  ativo, a frequência não fica totalmente fixa, e o clock medido ficou entre ~3,66 e 3,82 GHz
  nas duas etapas.
- **No relatório:** mencionar na metodologia.

### 4. Banda de memória da região `newton` (não otimizada, N = 2000 e 4000)

- Estouro do contador de 32 bits; corrigido ou omitido, sempre explicando (14, 17).

### 5. `-Wall` acrescentado às flags

- Só liga avisos e não muda o código gerado. Não precisa nem mencionar; se quiser, uma frase.
