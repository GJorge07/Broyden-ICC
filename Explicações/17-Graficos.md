# 17. Os 12 gráficos: o que fazer e como

## O que o enunciado exige

- **3 trechos × 4 testes = 12 gráficos de linhas.**
  - Trechos: `newton` (execução total), `jacobiana`, `sl`.
  - Testes: tempo, banda de memória, cache miss L2, MFLOP/s.
- Uma **cor por versão** (não otimizada × otimizada).
- **Eixo X = N em escala log**, com N = {32, 64, 128, 256, 512, 1000, 2000, 4000, 8000, 9000,
  10000, 20000}.
- No gráfico de **tempo**, o eixo Y também em **log**.
- **Cada gráfico tem de ser explicado** no relatório (ver 18).

| Teste | Grupo LIKWID | Métrica exata no arquivo | Eixo Y |
|---|---|---|---|
| Tempo | — (`tempos.csv`) | `total_ms`, `jacobiana_ms`, `sl_ms` | ms, **log** (obrigatório) |
| Banda de memória | `MEM` | `Memory bandwidth [MBytes/s]` | MB/s (sugestão: log) |
| Cache miss L2 | `L2CACHE` | `L2 miss ratio` | razão 0–1 (sugestão: linear) |
| Operações aritméticas | `FLOPS_DP` e `FLOPS_AVX` | `DP [MFLOP/s]` e `Packed DP [MFLOP/s]` | MFLOP/s (sugestão: log) |

## Onde estão os dados

```powershell
cd "C:\Users\Gabriel Jorge\Projetos\Trabalho ICC\broyden"
tar xzf resultados_coleta\resultados_nao-otimizado_likwid.tar.gz
tar xzf resultados_coleta\resultados_otimizado_likwid.tar.gz
```

Isso cria `resultados_nao-otimizado/` e `resultados_otimizado/` (ignoradas pelo git; não
comitar). Cada uma tem:
- `tempos.csv`: `versao,N,rep,total_ms,jacobiana_ms,sl_ms`, com 3 linhas por N;
- `raw/<versao>_<GRUPO>_<N>.txt`: saída do `likwid-perfctr`.

### Formato dos arquivos `raw`

```
Region jacobiana, Group 1: FLOPS_DP          <- começa o bloco da região
+-------------------+------------+
|    Region Info    | HWThread 3 |
| RDTSC Runtime [s] |   0.000014 |
|     call count    |         26 |
...
+----------------------+--------------+
|        Metric        |  HWThread 3  |
|       DP [MFLOP/s]   |    3245.0000 |          <- o valor que interessa
```

Como extrair: ler linha a linha. Ao ver `Region <nome>, Group`, guardar o nome da região. Nas
linhas com `|`, separar pelas barras (`split('|')`): a coluna 1 é o nome da métrica e a 2 é o
valor. Se o nome for a métrica procurada, guardar o valor de (versão, grupo, região, N). A
versão, o grupo e o N vêm do nome do arquivo.

Um leitor desse jeito foi testado em 19/11 e leu os 300 valores sem nenhuma falha.

## Ferramenta

O enunciado não exige ferramenta, mas o **script que gera os gráficos vai no pacote de
entrega** (`.py`, `.sh` ou `.pl`). Sugestão: Python + matplotlib.

```powershell
pip install matplotlib         # ainda não está instalado neste computador
```

Esqueleto do raciocínio (para escrever o script):
1. ler os dois `tempos.csv` e calcular, por (versão, N), a média (ou mediana) das 3
   repetições de cada coluna;
2. ler os `raw` como descrito acima;
3. para cada trecho e cada teste, fazer um gráfico com uma linha por versão:
   `plt.xscale('log')`, `plt.yscale('log')` quando for o caso, marcadores nos pontos,
   legenda, título, rótulos com unidade;
4. salvar como PDF ou PNG (PDF fica melhor no LaTeX, porque é vetorial).

## Decisões a tomar (e registrar no relatório)

1. **"Tempo médio".**
   - **(a) Média das 3 repetições do tempo somado das 25 iterações** (recomendado, é o valor
     que o programa imprime).
   - (b) A mesma média dividida pelo número de chamadas, para tempo por chamada. Atenção: a
     Jacobiana otimizada tem 26 chamadas (ver 06).

   Em log-log, (a) e (b) têm o mesmo formato, só deslocados.
2. **Média ou mediana** das repetições. A média é literalmente o que o enunciado pede; o
   ponto de N = 128 da otimizada fica um pouco alto (uma repetição lenta). A mediana elimina
   isso, mas precisa de justificativa. Recomendado: média, com comentário sobre N = 128.
3. **FLOPS_DP e FLOPS_AVX no mesmo gráfico** (são 4 gráficos por trecho, então os dois
   grupos dividem um). Sugestão: cor = versão, estilo = grupo (linha cheia para DP, tracejada
   para AVX). São 4 linhas.
4. **A versão não otimizada só vai até N = 4000.** A linha dela termina ali; explicar no texto
   e na legenda ou numa nota.
5. **MEM da região `newton` na não otimizada em N = 2000 e 4000:** valores errados por
   estouro do contador (ver 14). Opções:
   - corrigir (21,5 e 24,6 GB/s), avisando que foram corrigidos;
   - parar a linha em N = 1000.

   Não plotar os valores crus.
6. **Escala Y** dos testes que não são de tempo: livre. MEM e FLOPS variam ordens de grandeza
   entre as versões, então log ajuda; miss ratio fica melhor em linear.

## Dicas de apresentação

- Mesmas cores para a mesma versão em **todos** os gráficos.
- Ticks do eixo X nos próprios valores de N (`plt.xticks(Ns, Ns)`), não só nas potências de 10.
- Grade leve no fundo para facilitar a leitura em log.
- Um gráfico por figura, ou agrupar por trecho (4 subgráficos por figura) para ocupar menos
  páginas.
