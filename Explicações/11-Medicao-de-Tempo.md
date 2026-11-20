# 11. Medição de tempo: `utils.c` e o que entra em cada tempo

## `timestamp()`

```c
typedef double rtime_t;

rtime_t timestamp(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    return (rtime_t) tp.tv_sec * 1.0e3 + (rtime_t) tp.tv_nsec * 1.0e-6;   // ms
}
```

- **Não há `utils.*` pronto do professor**: o grupo escreve o seu. Este é o formato padrão da
  disciplina: `rtime_t` = `double`, em milissegundos.
- **`CLOCK_MONOTONIC_RAW`**: relógio monotônico, que nunca "volta no tempo" se o horário do
  sistema for ajustado (NTP). Com o `_RAW` também não sofre o ajuste gradual de frequência do
  NTP. É o mais adequado para medir intervalos.
- Mede **wall time** (tempo de relógio), não tempo de CPU.
- O `#ifndef CLOCK_MONOTONIC_RAW` serve só para compilar no Windows, onde essa constante não
  existe.

## O que entra em cada tempo

| Tempo impresso | Começa | Termina | Inclui |
|---|---|---|---|
| Total do Newton | antes da 1ª iteração (e, na otimizada, antes da `jacobianaTriConst`) | depois da última iteração | tudo: F, normas, Jacobianas, SLs, atualizações de x e histórico |
| Jacobianas | soma de cada chamada | — | otimizada: `jacobianaTriConst` (1×) + `jacobianaTri` (25×); não otimizada: `jacobianaDensa` (25×) |
| Sistemas lineares | soma de cada chamada | — | 25 chamadas do Gauss |

Não entram: leitura da entrada, alocação de `x` e `hist`, impressão.

## Tempos com e sem LIKWID

O script de coleta mede os **tempos** num binário **sem** marcadores e as **métricas** num
binário **com** marcadores. Assim o custo do LIKWID não contamina os tempos. Além disso,
dentro do código, os `LIKWID_MARKER_START/STOP` ficam **por fora** dos `timestamp()`.

## Resolução e ruído

Em N pequeno da versão otimizada, os tempos são de **microssegundos** (N = 32: 0,011 ms no
total). Nessa escala:
- uma interrupção do sistema já muda o resultado (N = 128: uma repetição 60 % mais lenta);
- o próprio `clock_gettime` custa algumas dezenas de nanossegundos.

Por isso cada medição foi repetida 3 vezes (ver 17 sobre média e mediana).
