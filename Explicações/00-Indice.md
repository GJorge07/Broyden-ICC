# Explicações do trabalho: índice

Material de estudo do grupo (Gabriel e Arthur). Cada arquivo explica uma parte do trabalho:
o que foi feito, como funciona, por que foi feito assim e onde está no código ou nos dados.
A pasta está no repositório para consulta, mas **não vai para a entrega** (ver 20).

> Substitui o antigo `Explicacoes.md` e as instruções de coleta que existiam em `likwid/`
> (estão no histórico do git, se precisar).

## Ordem de leitura

| # | Arquivo | Assunto |
|---|---|---|
| 01 | [01-Enunciado-e-Divisao.md](01-Enunciado-e-Divisao.md) | O que o enunciado pede, a divisão do grupo e a situação de cada item |
| 02 | [02-Problema-Broyden-e-Newton.md](02-Problema-Broyden-e-Newton.md) | A matemática: o sistema de Broyden, o Método de Newton e a Jacobiana |
| 03 | [03-Estrutura-Makefile-e-Compilacao.md](03-Estrutura-Makefile-e-Compilacao.md) | Arquivos, módulos, Makefile e flags |
| 04 | [04-Main-Entrada-e-Saida.md](04-Main-Entrada-e-Saida.md) | `main.c`: leitura, histórico e formato da saída |
| 05 | [05-Funcao-F-e-Norma.md](05-Funcao-F-e-Norma.md) | `broyden.c`: F(x) e a norma do máximo |
| 06 | [06-Jacobiana.md](06-Jacobiana.md) | `jacobiana.c`: Jacobiana densa e tridiagonal |
| 07 | [07-Sistemas-Lineares.md](07-Sistemas-Lineares.md) | `sistemas_lineares.c`: Gauss denso e Gauss tridiagonal (Arthur) |
| 08 | [08-Newton-Nao-Otimizado.md](08-Newton-Nao-Otimizado.md) | `newton_nao_otimizado.c`: a versão de referência |
| 09 | [09-Newton-Otimizado.md](09-Newton-Otimizado.md) | `newton.c`: a versão otimizada, linha a linha |
| 10 | [10-Otimizacoes-e-Slides.md](10-Otimizacoes-e-Slides.md) | Todas as otimizações: onde, como, por que e qual slide |
| 11 | [11-Medicao-de-Tempo.md](11-Medicao-de-Tempo.md) | `utils.c`: `timestamp()` e o que entra em cada tempo |
| 12 | [12-LIKWID-Instrumentacao.md](12-LIKWID-Instrumentacao.md) | Marcadores, `likwid_markers.h`, `make LIKWID=1`, grupos e métricas |
| 13 | [13-Script-de-Coleta.md](13-Script-de-Coleta.md) | `likwid/roda_testes.sh`, passo a passo |
| 14 | [14-Etapa-A.md](14-Etapa-A.md) | Coleta da versão não otimizada (18/11) |
| 15 | [15-Etapa-B.md](15-Etapa-B.md) | Coleta da versão otimizada (19/11) |
| 16 | [16-Verificacao-e-Testes.md](16-Verificacao-e-Testes.md) | Como se sabe que o programa está certo |
| 17 | [17-Graficos.md](17-Graficos.md) | Os 12 gráficos: o que o enunciado pede, de onde vêm os dados, como extrair |
| 18 | [18-Analise-dos-Graficos.md](18-Analise-dos-Graficos.md) | Os números de cada gráfico e como interpretar cada um |
| 19 | [19-Relatorio.md](19-Relatorio.md) | Estrutura do artigo e o que escrever em cada seção |
| 20 | [20-Entrega.md](20-Entrega.md) | Pacote final para o Moodle |
| 21 | [21-Decisoes-e-Desvios.md](21-Decisoes-e-Desvios.md) | Todas as decisões tomadas e os desvios do enunciado, com justificativa |
| 22 | [22-Perguntas-e-Estudo.md](22-Perguntas-e-Estudo.md) | Perguntas prováveis na entrevista e o que estudar para a prova |

## Onde está cada coisa no repositório

```
broyden/
├── Makefile, main.c, newton.c, newton_nao_otimizado.c, broyden.c/h,
│   jacobiana.c/h, sistemas_lineares.c/h, utils.c/h, likwid_markers.h, newton.h
├── likwid/
│   ├── roda_testes.sh             script de coleta (Linux)
│   ├── NOTAS_ETAPA_A.md           relatório da coleta da versão não otimizada
│   └── NOTAS_ETAPA_B.md           relatório da coleta da versão otimizada
├── resultados_coleta/
│   ├── resultados_nao-otimizado_likwid.tar.gz
│   └── resultados_otimizado_likwid.tar.gz
└── Explicações/                   (esta pasta)
```
