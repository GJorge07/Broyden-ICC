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
| Banda de memória | `MEM` | `Memory bandwidth [MBytes/s]` | MB/s, log |
| Cache miss L2 | `L2CACHE` | `L2 miss ratio` | razão 0–1, linear |
| Operações aritméticas | `FLOPS_DP` e `FLOPS_AVX` | `DP [MFLOP/s]` e `Packed DP [MFLOP/s]` | MFLOP/s, linear |

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

## Conferência dos 12 gráficos com o enunciado (20/11)

Abri os 12 PDFs um por um:

| Exigência do enunciado | Situação |
|---|---|
| 3 trechos × 4 testes = 12 gráficos de linhas | ✅ `tempo_`, `mem_`, `l2_`, `flops_` × `newton`, `jacobiana`, `sl` |
| Uma cor por versão | ✅ vermelho = não otimizado, azul = otimizado, em todos |
| Eixo X = N em escala log, com os 12 N | ✅ marcas em 32 … 20000 |
| Tempo: tempo médio em ms, eixo Y em log | ✅ |
| Banda: `Memory bandwidth [MBytes/s]` do grupo MEM | ✅ |
| Cache miss L2: miss ratio do grupo L2CACHE | ✅ `L2 miss ratio` |
| Operações: MFLOP/s de FLOPS_DP e FLOPS_AVX | ✅ `DP [MFLOP/s]` e `Packed DP [MFLOP/s]` no mesmo gráfico |
| MAX = 25, ε = 0 | ✅ registrado no `sistema.txt` das duas coletas |

Observações para o texto:
- a linha da não otimizada termina em N = 4000 (e em 1000 no `mem_newton`, com nota);
- em `flops_newton` a curva otimizada fica achatada perto de zero, e em `flops_jacobiana` a DP
  não otimizada também. Citar os valores no texto (tabela 4 do `18`).

O que é DP e AVX: ver `12-LIKWID-Instrumentacao.md`.

## Como os gráficos foram feitos

Python + matplotlib, em **dois passos**:

```
tempos.csv + raw/*.txt  --extrai_dados.py-->  graficos/dados.csv  --gera_graficos.py-->  graficos/figuras/*.pdf
```

Para rodar, na raiz do projeto:
```powershell
pip install matplotlib
tar xzf resultados_coleta/resultados_nao-otimizado_likwid.tar.gz
tar xzf resultados_coleta/resultados_otimizado_likwid.tar.gz
python graficos/extrai_dados.py
python graficos/gera_graficos.py
```

### `graficos/extrai_dados.py`

- `le_tempos(pasta)`: lê o `tempos.csv` e faz a **média das 3 repetições** de cada N. O
  `newton` usa a coluna `total_ms`, a `jacobiana` usa `jacobiana_ms` e o `sl` usa `sl_ms`.
- `le_metrica(arquivo, metrica)`: lê uma saída do LIKWID. Guarda a região atual ao ver
  `Region <nome>, Group` e, nas linhas de tabela (`| métrica | valor |`), pega o valor da
  métrica procurada. Devolve `{regiao: valor}`.
- Grava `graficos/dados.csv`, uma linha por (versão, região, N):
  `versao,regiao,N,tempo_ms,mem_mbs,l2_miss,dp_mflops,avx_mflops`. A não otimizada tem 8 N
  por região e a otimizada tem 12.
- **Para conferir qualquer ponto de um gráfico, basta abrir o `dados.csv`** (ele também está
  no git).

### `graficos/gera_graficos.py`

- `serie(...)`: separa do `dados.csv` os pontos (N, valor) de uma versão e de uma região.
- `grafico(...)`: gráfico com uma linha por versão (tempo, MEM e L2).
- `grafico_flops(...)`: gráfico com 4 linhas (DP e AVX de cada versão).
- `finaliza(...)`: eixos, legenda e grade, iguais em todos os gráficos; salva o PDF.
- Gera os 12 arquivos em `graficos/figuras/`: `tempo_<trecho>.pdf`, `mem_<trecho>.pdf`,
  `l2_<trecho>.pdf` e `flops_<trecho>.pdf`, com trecho = `newton`, `jacobiana` ou `sl`.

## Decisões tomadas (registrar no relatório)

| Decisão | Escolha | Motivo |
|---|---|---|
| "Tempo médio" | **Média das 3 repetições** do tempo que o programa imprime (soma das 25 iterações), em ms | É o que o enunciado pede e o valor que o programa mede |
| Média ou mediana | **Média** | Literal do enunciado. Comentar no texto que, em N = 128 da otimizada, uma repetição foi mais lenta e puxou o ponto para cima |
| MEM da região `newton`, não otimizada | **Linha termina em N = 1000**; nota no próprio gráfico | Em N = 2000 e 4000 o contador de 32 bits do grupo MEM estourou e o valor medido está errado (ver 14) |
| Formato | **12 PDFs separados** | Mais simples no script e no LaTeX; cada gráfico fica ao lado do seu texto |
| Cores | **Não otimizado = vermelho, otimizado = azul**, iguais em todos os gráficos | O enunciado pede uma cor por versão |
| Eixo X | **log**, com os 12 N do enunciado como marcas (rótulos na vertical, porque 8000, 9000 e 10000 ficam próximos em log) | Exigido pelo enunciado |
| Eixo Y do tempo | **log** | Exigido pelo enunciado |
| Eixo Y da banda (MEM) | **log** | Os valores vão de ~7 a ~126 000 MB/s; em linear, quase tudo ficaria colado no zero |
| Eixo Y do L2 miss ratio | **linear** | É uma razão entre 0 e 1 |
| Eixo Y do MFLOP/s | **linear** | O FLOPS_AVX vale **0** em alguns casos (`sl` otimizado, `jacobiana` não otimizada), e zero não aparece em escala log. Em linear, o zero fica visível e mostra a falta de vetorização |
| FLOPS_DP e FLOPS_AVX | **No mesmo gráfico**: cor = versão, linha cheia = DP, tracejada = AVX | São 4 gráficos por trecho; os dois grupos do teste de operações aritméticas dividem um |
| Não otimizada além de N = 4000 | A linha **termina em 4000** | Não foi medida (ver 14); explicar no texto |

## Dicas de apresentação

- Mesmas cores para a mesma versão em **todos** os gráficos.
- Ticks do eixo X nos próprios valores de N (`plt.xticks(Ns, Ns)`), não só nas potências de 10.
- Grade leve no fundo para facilitar a leitura em log.
- Um gráfico por figura, ou agrupar por trecho (4 subgráficos por figura) para ocupar menos
  páginas.
