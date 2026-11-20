# Broyden-ICC

Trabalho de CI1164 (Introdução à Computação Científica, UFPR): resolução de Sistemas
Não-Lineares de Broyden pelo Método de Newton, com versão otimizada e análise de desempenho
com LIKWID.

Autores: Gabriel Dobrowolski Jorge (GRR20250938) e Arthur Paul Nickel (GRR20252825).

## Organização

| Pasta / arquivo | Conteúdo |
|---|---|
| `*.c`, `*.h`, `Makefile` | código-fonte; `make` gera `broyden` (otimizado) e `broyden-nao-otimizado` |
| `likwid/roda_testes.sh` | script de coleta (tempo + LIKWID) usado no Lab-12/DINF |
| `likwid/NOTAS_ETAPA_*.md` | registro de cada coleta (condições, conferências, observações) |
| `resultados_coleta/*.tar.gz` | dados brutos das duas coletas (não alterar) |
| `graficos/` | scripts dos gráficos, `dados.csv` e as 12 figuras (`figuras/`) |
| `relatorio/` | relatório (`.docx` editável e `.pdf`) |
| `Explicações/` | material de estudo do grupo; começar por `00-Indice.md` |

## Uso

```bash
make
echo "1000 -1 0 25" | ./broyden          # entrada: N x0 epsilon MAX
```

Gráficos (na raiz, com Python + matplotlib):
```bash
tar xzf resultados_coleta/resultados_nao-otimizado_likwid.tar.gz
tar xzf resultados_coleta/resultados_otimizado_likwid.tar.gz
python graficos/extrai_dados.py
python graficos/gera_graficos.py
```
