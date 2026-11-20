# 18. Análise dos gráficos: os números e como interpretar

Valores extraídos dos dados da `h52` (são os mesmos de `graficos/dados.csv`, arredondados). Tempos = média das 3 repetições. "—" = não medido.
Use estas tabelas para **conferir** se os gráficos saíram certos e como base do texto.

Referências da máquina:
- L1d 32 kB, L2 256 kB, L3 6 MB;
- banda máxima teórica da DDR4 dual channel ≈ 38 GB/s;
- pico do núcleo (AVX2 + FMA, ~3,8 GHz) = 16 flops/ciclo ≈ 60 GFLOP/s.

Tamanho dos dados:

| | N = 128 | N = 512 | N = 1000 | N = 4000 | N = 20000 |
|---|---|---|---|---|---|
| Matriz densa (8N² B) | 128 kB (cabe na L2) | 2 MB (L3) | 8 MB (> L3) | 128 MB | 3,2 GB |
| Um vetor (8N B) | 1 kB | 4 kB | 8 kB | 32 kB | 160 kB |

---

## 1. Tempo (ms, eixo Y em log)

| N | newton não otim. | newton otim. | jacobiana não otim. | jacobiana otim. | sl não otim. | sl otim. |
|---|---|---|---|---|---|---|
| 32 | 0,120 | 0,0108 | 0,00485 | 0,00084 | 0,112 | 0,0070 |
| 64 | 0,670 | 0,0216 | 0,0290 | 0,00093 | 0,632 | 0,0136 |
| 128 | 4,75 | 0,0508 | 0,116 | 0,0054 | 4,61 | 0,0312 |
| 256 | 35,5 | 0,0871 | 0,493 | 0,0016 | 34,9 | 0,0527 |
| 512 | 273 | 0,217 | 2,01 | 0,0037 | 271 | 0,142 |
| 1000 | 2 396 | 0,393 | 8,10 | 0,0065 | 2 388 | 0,254 |
| 2000 | 30 454 | 0,842 | 32,8 | 0,0177 | 30 420 | 0,568 |
| 4000 | 268 954 | 1,54 | 120 | 0,0486 | 268 833 | 0,987 |
| 8000 | — | 2,89 | — | 0,0983 | — | 1,80 |
| 9000 | — | 3,25 | — | 0,115 | — | 2,03 |
| 10000 | — | 3,55 | — | 0,131 | — | 2,20 |
| 20000 | — | 6,64 | — | 0,298 | — | 4,23 |

**Como interpretar:**
- Em log-log, O(N^k) vira uma **reta de inclinação k**:
  - `sl` não otimizado: inclinação ~3 (×8 quando N dobra);
  - `jacobiana` não otimizada: ~2 (×4);
  - na otimizada, tudo ~1 (×2).
- O `newton` não otimizado praticamente coincide com o `sl`: o SL passa de 99 % do tempo a partir
  de N = 512 (99,2 % em 512, 99,96 % em 4000).
- O speedup cresce com N: ~11× em N = 32 e **~175 000×** em N = 4000.
- Na `jacobiana` otimizada, N pequeno (32–256) tem inclinação menor que 1 (o tempo cresce
  só ~1,9× enquanto N cresce 8×): um custo fixo (chamadas, `timestamp()`, laço muito curto),
  que não depende de N, domina quando o trabalho é de microssegundos. No `newton` e no `sl`
  a inclinação já é ~1 desde N = 32.
- Ponto fora da curva: `jacobiana` otimizada em N = 128 (uma repetição lenta, ver 15).
- Na otimizada, `newton` ≈ 1,5 × (`sl` + `jacobiana`): o resto do Newton (F, normas,
  atualização) também é O(N) e passa a aparecer.

---

## 2. Banda de memória (MB/s)

| N | newton não otim. | newton otim. | jacobiana não otim. | jacobiana otim. | sl não otim. | sl otim. |
|---|---|---|---|---|---|---|
| 32 | 36 | 111 | 4 494 | 126 200 (!) | 399 | 9 390 |
| 64 | 43 | 79 | 5 012 | 17 940 | 127 | 5 090 |
| 128 | 96 | 121 | 2 301 | 105 700 (!) | 196 | 3 077 |
| 256 | 25 | 82 | 445 | 15 130 | 22 | 1 313 |
| 512 | 10 | 71 | 398 | 13 230 | 6,7 | 600 |
| 1000 | 6 727 | 66 | 14 460 | 6 222 | 6 726 | 895 |
| 2000 | (omitido) | 70 | 23 990 | 3 491 | 21 520 | 179 |
| 4000 | (omitido) | 72 | 26 560 | 3 248 | 24 580 | 93 |
| 8000 | — | 122 | — | 1 410 | — | 58 |
| 9000 | — | 182 | — | 1 495 | — | 67 |
| 10000 | — | 254 | — | 2 827 | — | 90 |
| 20000 | — | 274 | — | 726 | — | 163 |

`newton` não otimizado em N = 2000 e 4000: os valores medidos (12 460 e 1 093) estão errados
por estouro do contador (ver 14). **No gráfico a linha termina em N = 1000**, com uma nota.
Se perguntarem: corrigindo as voltas perdidas daria ~21 500 e ~24 600 MB/s, praticamente
iguais ao `sl`.

**Como interpretar:**
- **Não otimizada, N ≤ 512:** a matriz cabe na cache (L2 até N = 128, L3 até N = 512), então
  quase nada vai para a RAM e a banda é baixa. **A partir de N = 1000** (8 MB > L3 de 6 MB) a
  matriz não cabe mais e cada iteração do Gauss passa por ela inteira: a banda **salta** para
  6,7 GB/s e chega a ~24,6 GB/s em N = 4000, perto do limite prático da máquina. Esse salto é
  o melhor gráfico para mostrar a hierarquia de memória.
- **Jacobiana densa** chega a 26,6 GB/s: ela só **escreve** (zera 128 MB por iteração em
  N = 4000). A banda é quase toda de "evict" (escrita de volta), o que mostra o
  write-allocate.
- **Otimizada:** os vetores de trabalho (4 × 8N B = 640 kB em N = 20000) cabem na cache, então
  o tráfego com a RAM é **desprezível**: o volume total da região `newton` em N = 20000 é só
  ~4,3 MB. Esse volume é praticamente o tamanho do histórico (26 × 8N B ≈ 4,2 MB), que é
  escrito uma vez e acaba indo para a RAM. Por isso a banda do `newton` **sobe** um pouco a
  partir de N = 8000 (66 → 274 MB/s), mas continua irrisória perto dos ~24 600 MB/s da não
  otimizada. Isso é bom: a versão otimizada não depende da memória.
- **Valores absurdos (!) na otimizada em N pequeno** (126 GB/s, acima do máximo físico da
  DDR4): **não são reais**. A região `jacobiana` otimizada dura poucos microssegundos no total
  e o contador MEM é do **socket inteiro**. Qualquer tráfego de fundo, ou um punhado de linhas
  de cache, dividido por um tempo minúsculo vira um número enorme. A métrica perde o sentido
  quando o volume é quase zero. No relatório: explicar isso, e se quiser mostrar o
  `Memory data volume [GBytes]`, que é praticamente zero.

---

## 3. L2 miss ratio

| N | newton não otim. | newton otim. | jacobiana não otim. | jacobiana otim. | sl não otim. | sl otim. |
|---|---|---|---|---|---|---|
| 32 | 0,23 | 0,20 | 0,23 | 0,20 | 0,22 | 0,19 |
| 64 | 0,20 | 0,22 | 0,22 | 0,23 | 0,16 | 0,21 |
| 128 | **0,022** | 0,24 | 0,21 | 0,23 | **0,010** | 0,24 |
| 256 | 0,21 | 0,24 | 0,22 | 0,23 | 0,21 | 0,22 |
| 512 | 0,23 | 0,24 | 0,31 | 0,23 | 0,23 | 0,25 |
| 1000 | 0,25 | 0,24 | 0,28 | 0,24 | 0,25 | 0,25 |
| 2000 | 0,22 | 0,23 | 0,25 | 0,23 | 0,22 | 0,24 |
| 4000 | 0,20 | 0,23 | 0,25 | 0,26 | 0,20 | 0,24 |
| 8000 | — | 0,25 | — | 0,28 | — | 0,26 |
| 9000 | — | 0,27 | — | 0,30 | — | 0,27 |
| 10000 | — | 0,27 | — | 0,30 | — | 0,28 |
| 20000 | — | 0,29 | — | 0,31 | — | 0,29 |

**Como interpretar:**
- Valores de **~0,2–0,3** são típicos de acesso **sequencial**: uma linha de cache tem 8
  doubles, então só o 1º acesso de cada linha erra. A razão não chega a 1/8 porque o
  prefetcher e as escritas também contam.
- **Queda forte na não otimizada em N = 128** (0,01 no `sl`): a matriz de 128 kB **cabe
  inteira na L2** de 256 kB. Depois da primeira passada, quase todo acesso acerta na L2. Em
  N = 256 (512 kB) não cabe mais e a razão volta a ~0,2. É o efeito do tamanho do conjunto de
  dados ("working set") em relação à cache.
- **Jacobiana densa com pico em N = 512** (0,31): matriz de 2 MB; a escrita de muitas linhas
  novas gera misses de write-allocate.
- **Otimizada sobe devagar com N** (0,20 → 0,29): os vetores crescem; em N = 20000 cada vetor
  tem 160 kB e os 4 do Newton (640 kB) já não cabem juntos na L2.
- **Miss ratio parecido não significa desempenho parecido.** É uma razão: a otimizada faz
  milhões de vezes menos acessos. O número de misses absoluto é muito menor.

---

## 4. MFLOP/s (FLOPS_DP = todas as larguras; FLOPS_AVX = só AVX de 256 bits)

| N | newton não otim. DP / AVX | newton otim. DP / AVX | jacobiana não otim. DP / AVX | jacobiana otim. DP / AVX | sl não otim. DP / AVX | sl otim. DP / AVX |
|---|---|---|---|---|---|---|
| 32 | 172 / 146 | 5,2 / 1,9 | 171 / 0 | 412 / 375 | 5 120 / 4 112 | 747 / 0 |
| 64 | 1 149 / 1 031 | 9,8 / 4,1 | 101 / 0 | 781 / 630 | 8 087 / 7 002 | 954 / 0 |
| 128 | 4 264 / 4 261 | 19 / 8,3 | 52 / 0 | 1 365 / 1 180 | 7 683 / 8 016 | 1 040 / 0 |
| 256 | 7 269 / 7 007 | 39 / 16 | 26 / 0 | 2 030 / 1 653 | 8 187 / 7 858 | 1 039 / 0 |
| 512 | 8 171 / 8 072 | 77 / 32 | 13 / 0 | 3 712 / 2 764 | 8 344 / 8 242 | 856 / 0 |
| 1000 | 6 960 / 6 845 | 137 / 61 | 6,0 / 0 | 3 245 / 3 664 | 6 999 / 6 883 | 969 / 0 |
| 2000 | 4 401 / 4 365 | 243 / 105 | 3,0 / 0 | 3 400 / 3 550 | 4 406 / 4 371 | 804 / 0 |
| 4000 | 3 958 / 3 952 | 402 / 179 | 1,6 / 0 | 3 710 / 3 634 | 3 960 / 3 954 | 983 / 0 |
| 8000 | — | 643 / 271 | — | 3 932 / 3 770 | — | 1 030 / 0 |
| 9000 | — | 687 / 291 | — | 3 693 / 3 747 | — | 1 026 / 0 |
| 10000 | — | 711 / 309 | — | 3 821 / 3 762 | — | 1 078 / 0 |
| 20000 | — | 997 / 437 | — | 3 544 / 3 412 | — | 1 072 / 0 |

(DP e AVX vêm de **execuções diferentes**, uma por grupo; por isso AVX às vezes passa
de DP, até ~13 % nas regiões de microssegundos, como a `jacobiana` otimizada em N = 1000: as
instruções contadas são as mesmas, só o tempo medido da região muda.)

**Como interpretar:**
- **`sl` não otimizado**: sobe até **~8,3 GFLOP/s** em N = 256–512 (matriz na cache, laço
  interno vetorizado) e **cai** para ~4,0 GFLOP/s em N = 4000, quando a matriz sai da L3 e a
  memória vira o gargalo. É o mesmo ponto em que a banda (gráfico 2) sobe. **AVX ≈ DP**:
  quase todas as contas são vetorizadas.
- **`sl` otimizado**: ~1 GFLOP/s constante e **AVX = 0**. O Gauss tridiagonal é escalar
  (recorrência, ver 07).
- **Contradição aparente (o ponto mais importante do relatório):** o `sl` otimizado tem
  **4× menos MFLOP/s** (em N = 4000) e é **~270 000× mais rápido**. MFLOP/s mede o **ritmo** das contas,
  não a velocidade da solução. A versão otimizada faz ~8N flops por chamada em vez de ~⅔N³.
  Fazer muito menos trabalho vale mais do que fazer o trabalho rápido: MFLOP/s sozinho não
  diz qual versão é melhor; quem decide é o tempo.
- **Jacobiana não otimizada**: MFLOP/s **cai** com N (171 → 1,6) e AVX = 0. Ela faz 2N flops
  úteis, mas escreve N² posições; o tempo cresce com N² e os flops com N.
- **Jacobiana otimizada**: ~3,5 GFLOP/s, quase tudo **AVX** (vetorizada, sem `if` e com
  `restrict`).
- **`newton` otimizado em N pequeno** (5 MFLOP/s em N = 32): o custo dos 51 pares de
  START/STOP do LIKWID dentro da região é maior que o próprio cálculo. Sobe com N à medida que
  o cálculo passa a dominar. Na não otimizada o efeito é igual em N = 32 (172 MFLOP/s).
- **Pico teórico ~60 GFLOP/s**: nenhuma região chega perto. O `sl` denso fica em ~14 % do
  pico; o limite real está em carregar e gravar a matriz.

---

## Resumo do que o relatório deve mostrar

1. A troca de estrutura de dados e de algoritmo muda a **complexidade** (retas de inclinação 3
   → 1 no tempo).
2. A versão não otimizada passa de limitada pela cache/cálculo para **limitada pela memória**
   quando a matriz sai da L3 (N = 1000): a banda sobe e o MFLOP/s cai.
3. A versão otimizada **cabe na cache** em todos os N: banda desprezível, miss ratio de acesso
   sequencial.
4. **MFLOP/s menor não é pior:** o que importa é o tempo para resolver.
5. Anomalias explicadas: estouro do MEM (N = 2000/4000), banda absurda em regiões de
   microssegundos, custo dos marcadores em N pequeno, N = 128 cabendo na L2.
