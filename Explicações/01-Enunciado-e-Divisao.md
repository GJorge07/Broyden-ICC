# 01. Enunciado e divisão do trabalho

## O que o enunciado pede (CI1164, Profs. Armando Delgado e Guilherme Derenievicz)

Implementar de forma **otimizada** um programa `broyden` que resolve o **Sistema Não-Linear de
Broyden** pelo **Método de Newton**, e **avaliar o desempenho** com o LIKWID.

Entregar:
1. o código-fonte em C;
2. um **relatório em formato de artigo científico**, com a análise de desempenho e a
   justificativa das otimizações.

### Programa

| Exigência | Como foi atendida |
|---|---|
| Lê da entrada padrão `N x0 epsilon MAX` | `main.c`, um `scanf` |
| Imprime todas as iterações, cada incógnita em uma linha (`x1 = ...`), separadas por `#` | `main.c`, a partir do histórico `hist` |
| No final, entre linhas `###########`: tempo total do Newton, soma dos tempos das Jacobianas e soma dos tempos dos sistemas lineares | `main.c`, campos de `TemposNewton` |
| Tempos em ms com `timestamp()` dos `utils.*` | `utils.c` (o grupo escreve o seu; não há arquivo do professor) |
| Código modular, `main()` sozinha em um módulo | `main.c` só tem a `main` |
| Funções comentadas | Todas têm comentário no `.c` e no `.h` |
| Makefile com `all` (gera `broyden`) e `clean` e compilação dos módulos separadamente | `Makefile` |
| Executado como `./broyden` | Sim |

### Otimização

Aplicar técnicas vistas em aula e explicar no relatório **qual otimização**, **em qual função
ou trecho** e **por que melhora o desempenho**. É "aconselhável" comparar versões com e sem
otimização em gráficos de linhas. Perguntas que o enunciado sugere responder:
- Qual método usar nos sistemas lineares internos? → Gauss tridiagonal (ver 07).
- Qual estrutura de dados para matrizes e vetores? → Jacobiana em 3 vetores (ver 06).
- Como calcular a Jacobiana a cada iteração? → só a diagonal muda (ver 06 e 09).

### Análise de desempenho

- Trechos analisados: **Newton total**, **cálculo das Jacobianas** e **resolução dos sistemas
  lineares**. Um marcador LIKWID para cada um.
- N = {32, 64, 128, 256, 512, 1000, 2000, 4000, 8000, 9000, 10000, 20000}, eixo X em log.
- MAX = 25 e ε = 0 em todos os testes.
- 4 testes por trecho: tempo (Y em log também), banda de memória (MEM), cache miss L2
  (L2CACHE) e MFLOP/s (FLOPS_DP e FLOPS_AVX). São 12 gráficos, uma cor por versão.
- Cada gráfico tem de ser explicado.

### Condições "imprescindíveis"

- GCC com `-O3 -march=native -mavx -fopt-info-vec`.
- Compilar na mesma máquina dos testes.
- Mesmos parâmetros e igualdade de condições.
- Marcadores LIKWID em cada trecho.
- Não usar as servidoras compartilhadas do DINF.
- Mostrar a arquitetura do processador (`likwid-topology -g -c`).

### Entrega

Grupo de até 2 alunos. Pacote `login1-login2.tar.gz` (ou `.zip`) com a pasta `login1-login2/`
contendo `Makefile`, `.c`, `.h`, scripts (`.sh`, `.py`) e `relatorio-login1-login2.pdf`. Ver 20.

## Divisão do grupo

| Quem | Parte |
|---|---|
| **Gabriel** | Estudo; Jacobiana; laço do Newton; instrumentação e coleta com LIKWID; seções de arquitetura, metodologia e otimizações; gráficos e interpretação |
| **Arthur** | Solver otimizado (Gauss para matriz tridiagonal / k-diagonal); revisão final |
| **Apoio (Claude)** | Versão não otimizada, Makefile, entrada/saída, script de coleta, documentação |

## Situação (19/11/2026)

| Item | Estado |
|---|---|
| Código das duas versões | ✅ Pronto e conferido |
| Solver do Arthur | ✅ Pronto e conferido |
| Coleta LIKWID, Etapa A (não otimizada) | ✅ |
| Coleta LIKWID, Etapa B (otimizada) | ✅ |
| 12 gráficos | ⏳ Gabriel |
| Relatório | ⏳ Gabriel, com revisão do Arthur |
| Pacote de entrega | ⏳ |
