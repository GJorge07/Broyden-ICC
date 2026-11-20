# 19. Relatório: estrutura do artigo e o que escrever

Formato de **artigo científico** (o enunciado não fixa o modelo; um template de artigo em
LaTeX, por exemplo o da SBC, funciona bem). Nome do arquivo:
**`relatorio-gdj25-apn25.pdf`**.

## Estrutura sugerida

### Título, autores e resumo
- Título curto: por exemplo, "Otimização do Método de Newton para o Sistema Não-Linear de
  Broyden".
- Autores: Gabriel Dobrowolski Jorge (gdj25) e Arthur Paul Nickel (apn25), UFPR.
- Resumo (5–8 linhas): problema, o que foi otimizado, onde foi medido e o resultado principal
  (por exemplo, speedup de ~175 000× em N = 4000; complexidade O(n³) → O(n)).

### 1. Introdução
- O sistema de Broyden, o Método de Newton e o objetivo do trabalho.
- Organização do texto.

### 2. O problema e o método (base: 02)
- As equações, o Newton, a Jacobiana tridiagonal (com a derivação) e os critérios de parada.

### 3. Implementação e otimizações (base: 06, 07, 09, 10)
Para **cada** otimização: **o que é**, **onde** (função) e **por que** melhora (o enunciado pede
os três). Ordem sugerida, da mais para a menos importante:
1. estrutura de dados (3 vetores);
2. algoritmo (Gauss tridiagonal, sem pivoteamento e por quê, com o truque do inverso);
3. loop-invariant code motion (`jacobianaTriConst`);
4. −F direto e fatoração;
5. fusão de laços, **com a contrapartida** da vetorização perdida;
6. laços sem desvio + `restrict` + `inline` + alinhamento, confirmados pelo `-fopt-info-vec`.

Mencionar também o que **não** foi aplicado e por quê (unroll & jam e blocking não se aplicam
a vetores 1D; `-ffast-math` proibido).

### 4. Metodologia experimental (base: 11, 12, 13, 14, 15)
- **Arquitetura** (obrigatória, do `topologia.txt`): i5-7500 @ 3,40 GHz, 4 núcleos, sem SMT;
  L1d 32 kB / L2 256 kB por núcleo / L3 6 MB compartilhada; linha de 64 B. Uma tabela.
- Ambiente: `h52` (Lab-12/DINF), Debian, kernel 6.12.107, gcc 14.2.0, LIKWID 5.5.2.
- Flags: `-O3 -march=native -mavx -fopt-info-vec` (ver 21 sobre a Etapa A).
- Núcleo 3 (`taskset` e `likwid-perfctr -C 3`), governor `performance`, **turbo ligado**.
- Parâmetros: N, x0 = −1 (e por quê), ε = 0, MAX = 25.
- Tempo: `timestamp()` com `CLOCK_MONOTONIC_RAW`; binário **sem** marcadores; 3 repetições;
  estatística usada (média).
- Métricas: binário **com** marcadores; regiões `newton`, `jacobiana`, `sl`; grupos e métricas
  exatas.
- Versão não otimizada até N = 4000, com a justificativa (estimativa de horas a dias).
- Corretude: as duas versões coincidem até 1e−15.

### 5. Resultados e análise (base: 18)
- Os 12 gráficos, agrupados por trecho ou por teste.
- **Cada gráfico com um parágrafo de explicação** (exigência do enunciado).
- Explicar as anomalias: estouro do MEM, banda em regiões de microssegundos, N = 128 cabendo
  na L2, custo dos marcadores em N pequeno.
- Discussão central: MFLOP/s × tempo.

### 6. Conclusão
- O ganho veio principalmente de explorar a estrutura do problema (dados + algoritmo); as
  micro-otimizações complementam.
- A versão otimizada fica na cache e é O(n).
- Limitações: N da não otimizada, turbo ligado, uma execução por grupo do LIKWID.

### Referências
Material da disciplina (Partes 8, 9 e 10), documentação do LIKWID e um livro de cálculo
numérico (para o Newton e o Gauss).

## Divisão sugerida

| Seção | Quem escreve | Base |
|---|---|---|
| 1, 2, 4, 5, 6 | Gabriel | arquivos 02, 11–18 |
| 3 (dados, Newton, micro-otimizações) | Gabriel | 06, 09, 10 |
| 3 (Gauss tridiagonal) | Arthur ou Gabriel | 07 |
| Revisão geral | Arthur | — |
