# 15. Etapa B: coleta da versão otimizada (19/11/2026)

Detalhes completos: `likwid/NOTAS_ETAPA_B.md`. Dados:
`resultados_coleta/resultados_otimizado_likwid.tar.gz`.

## Condições

Iguais às da Etapa A (mesma `h52`, núcleo 3, governor `performance`, turbo ligado, kernel,
gcc 14.2.0, LIKWID 5.5.2, x0 = −1, ε = 0, MAX = 25, 3 repetições), com duas diferenças:
- **flags corretas** do enunciado: `-O3 -march=native -mavx -fopt-info-vec`;
- **todos os 12 valores de N** (32 a 20000).

Código: commit `36d095a` (solver do Arthur incluído). A coleta levou poucos minutos.

## Conferências feitas no Linux (todas passaram)

| Conferência | Resultado |
|---|---|
| `sistema.txt` igual ao da Etapa A | ✅ exceto a linha `Flags` (`-mavx2` → `-mavx`) |
| `topologia.txt` igual ao da Etapa A | ✅ exceto "Free memory", que só registra o momento |
| `tempos.csv` | ✅ 36 linhas, nenhum campo vazio |
| `raw/` | ✅ 48 arquivos, todos com as 3 regiões, sem erro de MSR |
| Estouro do MEM | ✅ nenhum: volume `newton` ≥ `sl` + `jacobiana` em todos os N (máximo 4,3 MB) |
| Comparação entre versões antes de medir | ✅ diferença máxima 9,99e−16 |

No Windows, em 19/11, conferi de novo: os **300 valores** necessários para os gráficos (2
versões, 3 regiões, 4 grupos, todos os N) foram lidos dos `raw` sem nenhuma falha.

## Tempos médios (ms)

| N | Total | Jacobianas | SL |
|---|---|---|---|
| 32 | 0,0108 | 0,00084 | 0,0070 |
| 64 | 0,0216 | 0,00093 | 0,0136 |
| 128 | 0,0508 | 0,0054 (*) | 0,0312 |
| 256 | 0,0871 | 0,0016 | 0,0527 |
| 512 | 0,217 | 0,0037 | 0,142 |
| 1000 | 0,393 | 0,0065 | 0,254 |
| 2000 | 0,842 | 0,0177 | 0,568 |
| 4000 | 1,54 | 0,0486 | 0,987 |
| 8000 | 2,89 | 0,0983 | 1,80 |
| 9000 | 3,25 | 0,115 | 2,03 |
| 10000 | 3,55 | 0,131 | 2,20 |
| 20000 | 6,64 | 0,298 | 4,23 |

(*) Em N = 128, a 3ª repetição foi lenta (Jacobiana 0,0139 ms contra ~0,0011 nas outras). Com
tempos de microssegundos, uma interrupção do sistema causa isso. A média fica puxada para cima
(variação de 52 % entre as repetições). Ver 17.

## Comparação com a Etapa A

| N | Não otimizada | Otimizada | Speedup |
|---|---|---|---|
| 32 | 0,120 ms | 0,0108 ms | ~11× |
| 256 | 35,5 ms | 0,0871 ms | ~410× |
| 1000 | 2 396 ms | 0,393 ms | ~6 100× |
| 4000 | 268 954 ms | 1,54 ms | **~175 000×** |

O speedup cresce com N porque as complexidades são diferentes: n³/n = n². Ele não é uma
constante; quanto maior o problema, maior a vantagem.
