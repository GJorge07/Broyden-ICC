# 20. Entrega no Moodle

## Estrutura exigida

```
gdj25-apn25/
├── Makefile                         make gera ./broyden
├── *.c, *.h                         código-fonte
├── *.sh, *.py                       scripts de coleta (LIKWID) e de gráficos (podem ir em subpastas)
└── relatorio-gdj25-apn25.pdf        relatório
```

Compactado como **`gdj25-apn25.tar.gz`** (ou `.zip`). Logins: `gdj25` (Gabriel) e `apn25`
(Arthur), tirados dos commits.

## O que entra

| Item | Origem |
|---|---|
| `Makefile`, `main.c`, `newton.c`, `newton_nao_otimizado.c`, `newton.h`, `broyden.c/h`, `jacobiana.c/h`, `sistemas_lineares.c/h`, `utils.c/h`, `likwid_markers.h` | raiz do repositório |
| `likwid/roda_testes.sh` | script de coleta |
| `graficos/extrai_dados.py` e `graficos/gera_graficos.py` | scripts dos gráficos (prontos) |
| `relatorio-gdj25-apn25.pdf` | a fazer |

## O que não entra

- `Explicações/` (material de estudo) e `likwid/NOTAS_ETAPA_*.md` (registros da coleta);
- `Duvidas.md` e `image.png` (pessoais, nem estão no git);
- `.o`, executáveis, pastas `resultados_*/` extraídas e `.git`.

Os `.tar.gz` de resultados são opcionais. O enunciado não pede os dados brutos; incluir em
`dados/` ajuda a reproduzir os gráficos, mas aumenta o pacote. Decidir com o Arthur.

## Conferência antes de enviar

```bash
mkdir /tmp/teste && tar xzf gdj25-apn25.tar.gz -C /tmp/teste && cd /tmp/teste/gdj25-apn25
make && echo "4 -1 0 25" | ./broyden && make clean && ls
```
Tem que compilar sem erro, rodar e o `make clean` tem que deixar só os fontes.
