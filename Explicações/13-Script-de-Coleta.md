# 13. `likwid/roda_testes.sh`: o script de coleta

Automatiza toda a medição no Linux, para que ir até o laboratório fosse só rodar um comando.
Entra na entrega como "script de processamento de dados do LIKWID".

## Uso

```bash
./likwid/roda_testes.sh teste                     # rápido: N = 32 e 64
VERSOES=nao-otimizado ./likwid/roda_testes.sh     # Etapa A
VERSOES=otimizado     ./likwid/roda_testes.sh     # Etapa B
./likwid/roda_testes.sh                           # as duas versões de uma vez
```

Variáveis opcionais: `NMAX_NAO_OTIM` (padrão 4000), `REPS` (3), `CORE` (3), `VERSOES`.
Fixos: x0 = −1, ε = 0, MAX = 25.

## Como rodar no DINF (o que foi feito nas duas etapas)

```bash
ssh <login>@macalan.c3sl.ufpr.br      # só passagem: não medir na macalan
ssh h52                                # máquina do Lab-12 usada nas duas etapas
git clone https://github.com/GJorge07/Broyden-ICC.git   # ou git pull, se já existir
cd Broyden-ICC
tmux new -s coleta                     # se a conexão cair, a coleta continua
VERSOES=otimizado ./likwid/roda_testes.sh 2>&1 | tee coleta.log
```
Não precisa de `sudo` no DINF. Se der `bad interpreter` (CRLF): `sed -i 's/\r$//' likwid/roda_testes.sh`.

## O que faz, em ordem

1. **Verifica o LIKWID** (`likwid-perfctr` no PATH) e descobre o `LIKWID_HOME` a partir dele
   (no DINF, `/home/soft/likwid`).
2. **Prepara o sistema:**
   - usa `sudo` só se o usuário tiver permissão (no DINF não tem, e o script segue com um
     aviso);
   - carrega o módulo `msr` e ajusta `perf_event_paranoid` quando possível;
   - **fixa a frequência**: `likwid-setFrequencies -g performance -c 3`. Um `trap` devolve para
     `powersave` no final, mesmo se o script for interrompido.
3. **Registra o ambiente** em `sistema.txt` (data, kernel, gcc, flags, governor, turbo, núcleo,
   parâmetros, máquina, versão do LIKWID, frequências, `uptime` e `who` para registrar se a
   máquina estava exclusiva, `lscpu`). O `uptime`/`who` foi acrescentado depois das coletas,
   então não aparece nos `sistema.txt` já coletados e a topologia em
   `topologia.txt` (`likwid-topology -c -g`, exigido para a seção Arquitetura).
4. **Escolhe os grupos:** MEM (ou L3), L2CACHE (ou L1CACHE/CACHE), FLOPS_DP, FLOPS_AVX. Na
   `h52` todos existem, então nenhuma alternativa foi usada.
5. **Compila duas vezes:**
   - `make` → binários **sem** marcadores, para medir tempo;
   - `make LIKWID=1` → binários **com** marcadores, para as métricas.

   Guarda a saída de cada compilação (`compilacao.log` e `compilacao_likwid.log`, com o
   `-fopt-info-vec`).
6. **Testes de sanidade antes de medir:**
   - se a versão otimizada estiver incluída: compara as saídas das duas versões em N = 32
     (tolerância 1e−8) e **aborta** se não baterem, para não medir um solver errado;
   - roda o LIKWID uma vez e confere se as 3 regiões aparecem;
   - confere se os contadores **não vieram zerados** (`CPU_CLK_UNHALTED_CORE` > 0). Na
     `macalan` vinham zerados.
7. **Coleta:** para cada versão e cada N (a não otimizada para em `NMAX_NAO_OTIM`):
   - **3 medições de tempo** com `taskset -c 3`, gravadas em `tempos.csv`;
   - **1 execução por grupo** com `likwid-perfctr -C 3 -g GRUPO -m`, saída em
     `raw/<versao>_<GRUPO>_<N>.txt` (as linhas `x = ...` e `#` são filtradas).
8. **Empacota** a pasta em `resultados_<versao>_likwid.tar.gz`.

## Por que 3 repetições de tempo e só 1 do LIKWID

- O tempo é a métrica mais sensível a ruído e o enunciado fala em "tempo médio".
- As métricas do LIKWID são razões (MFLOP/s, MB/s, miss ratio) de uma execução de 25
  iterações; repetir também seria bom, mas na não otimizada cada execução de N = 4000 leva
  ~4,5 min. Com 3 repetições de tempo + 4 grupos, já são 7 execuções por N.

## Formato do `tempos.csv`

```
versao,N,rep,total_ms,jacobiana_ms,sl_ms
otimizado,32,1,0.01090000,0.00083800,0.00710000
...
```
