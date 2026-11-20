#!/bin/bash
# =============================================================================
# Coleta de dados de desempenho (tempo + LIKWID) do trabalho Broyden/Newton.
#
# Uso (a partir da pasta raiz do projeto):
#   ./likwid/roda_testes.sh teste    -> teste rapido (N=32 e 64), ~1-2 min
#   ./likwid/roda_testes.sh          -> coleta completa (~1 h)
#
# Variaveis opcionais (ex.: NMAX_NAO_OTIM=2000 ./likwid/roda_testes.sh):
#   NMAX_NAO_OTIM  maior N da versao nao otimizada (padrao 4000)
#   REPS           repeticoes de cada medicao de tempo (padrao 3)
#   CORE           nucleo onde o programa roda (padrao 3, exigido no Lab-12)
#   VERSOES        quais versoes medir (padrao "otimizado nao-otimizado").
#                  Ex.: VERSOES=nao-otimizado ./likwid/roda_testes.sh
#                  mede so a nao otimizada (nao depende do solver do Arthur)
#                  e salva em resultados_nao-otimizado/.
#
# Saida: pasta resultados/ e o arquivo resultados_likwid.tar.gz
# =============================================================================
set -u

cd "$(dirname "$0")/.." || exit 1

MODO="${1:-completo}"
NMAX_NAO_OTIM="${NMAX_NAO_OTIM:-4000}"
REPS="${REPS:-3}"
CORE="${CORE:-3}"   # enunciado: Lab-12/DINF usar SEMPRE o core 3
VERSOES="${VERSOES:-otimizado nao-otimizado}"
X0=-1         # chute inicial (com 0.5 o Newton nao converge)
EPS=0          # exigido pelo enunciado
MAXIT=25       # exigido pelo enunciado

if [ "$MODO" = "teste" ]; then
    NS="32 64"
    OUT="resultados_teste"
else
    NS="32 64 128 256 512 1000 2000 4000 8000 9000 10000 20000"
    OUT="resultados"
fi

# Rodando so uma das versoes: pasta de saida propria (nao apaga a outra)
for V in $VERSOES; do
    case "$V" in otimizado|nao-otimizado) ;; *) echo "VERSOES invalida: $V"; exit 1;; esac
done
if [ "$VERSOES" != "otimizado nao-otimizado" ]; then
    OUT="${OUT}_$(echo $VERSOES | tr ' ' '+')"
fi

log()  { echo "[$(date +%H:%M:%S)] $*"; }
erro() { echo; echo "ERRO: $*"; echo "Veja Explicações/13-Script-de-Coleta.md"; exit 1; }

rm -rf "$OUT" bin
mkdir -p "$OUT/raw" bin

# -----------------------------------------------------------------------------
# 1. Verificacoes e preparacao do sistema (slides 17 e 23 da Parte 8)
# -----------------------------------------------------------------------------
log "Verificando o LIKWID..."
command -v likwid-perfctr >/dev/null || erro "likwid-perfctr nao encontrado. Instale: sudo apt install likwid"

# Compila com o likwid.h/liblikwid da MESMA instalacao do likwid-perfctr em uso
# (no DINF: /home/soft/likwid, slide 15). Se nao achar, o Makefile usa /usr/local.
if [ -z "${LIKWID_HOME:-}" ]; then
    LH="$(dirname "$(dirname "$(command -v likwid-perfctr)")")"
    [ -f "$LH/include/likwid.h" ] && LIKWID_HOME="$LH"
fi

# sudo so e usado se o usuario tiver permissao (ex.: no proprio notebook).
# Em maquinas sem sudo (ex.: laboratorio via ssh) os passos abaixo sao
# pulados com um aviso, em vez de travar pedindo uma senha inexistente.
if sudo -n true 2>/dev/null || id -nG | grep -qwE 'sudo|wheel|admin'; then
    PODE_SUDO=1
else
    PODE_SUDO=0
    log "Sem sudo (normal no DINF): msr/perf_event_paranoid ficam como a maquina ja configura"
fi
tenta_sudo() { [ "$PODE_SUDO" = 1 ] && sudo "$@"; }

if ! lsmod | grep -q '^msr'; then
    log "Carregando o modulo msr (pede a senha do sudo)..."
    tenta_sudo modprobe msr || log "AVISO: nao consegui carregar o modulo msr"
fi
if [ "$(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null)" != "-1" ]; then
    tenta_sudo sysctl -q -w kernel.perf_event_paranoid=-1 2>/dev/null \
        || log "AVISO: perf_event_paranoid = $(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null) (nao consegui mudar para -1)"
fi

# Enunciado: "likwid-setFrequencies -g performance -c 3" antes dos experimentos
# e "likwid-setFrequencies -g powersave -c 3" depois. Usa o nucleo CORE.
log "Fixando a frequencia do nucleo $CORE no modo 'performance'..."
FREQ_FIXADA=0
if command -v likwid-setFrequencies >/dev/null && \
   { likwid-setFrequencies -g performance -c "$CORE" >/dev/null 2>&1 || \
     tenta_sudo likwid-setFrequencies -g performance -c "$CORE" >/dev/null 2>&1; }; then
    FREQ_FIXADA=1
elif [ "$(cat /sys/devices/system/cpu/cpu$CORE/cpufreq/scaling_governor 2>/dev/null)" = "performance" ]; then
    log "Governor ja esta em 'performance'"
elif command -v cpupower >/dev/null && tenta_sudo cpupower -c "$CORE" frequency-set -g performance >/dev/null 2>&1; then
    log "Governor fixado com cpupower"
else
    log "AVISO: nao consegui fixar a frequencia (rode: likwid-setFrequencies -g performance -c $CORE)"
fi
# Ao terminar (ou se o script for interrompido), volta para powersave
volta_freq() {
    if [ "$FREQ_FIXADA" = 1 ]; then
        likwid-setFrequencies -g powersave -c "$CORE" >/dev/null 2>&1 \
            || tenta_sudo likwid-setFrequencies -g powersave -c "$CORE" >/dev/null 2>&1
        log "Frequencia do nucleo $CORE devolvida para 'powersave'"
    fi
}
trap volta_freq EXIT

# Informacoes do sistema para o relatorio (slide 22: documentar tudo)
{
    echo "Data: $(date)"
    echo "Kernel: $(uname -a)"
    echo "Compilador: $(gcc --version | head -1)"
    echo "Flags: -O3 -march=native -mavx -fopt-info-vec"
    echo "Governor: $(cat /sys/devices/system/cpu/cpu$CORE/cpufreq/scaling_governor 2>/dev/null)"
    echo "Turbo desabilitado (intel_pstate/no_turbo): $(cat /sys/devices/system/cpu/intel_pstate/no_turbo 2>/dev/null)"
    echo "Nucleo usado: $CORE"
    echo "Parametros: x0=$X0 eps=$EPS MAX=$MAXIT REPS=$REPS NMAX_NAO_OTIM=$NMAX_NAO_OTIM"
    echo "Maquina: $(hostname)"
    echo "LIKWID: $(likwid-perfctr -v 2>&1 | head -1) ($(command -v likwid-perfctr))"
    echo "LIKWID_HOME usado na compilacao: ${LIKWID_HOME:-/usr/local}"
    echo
    echo "--- likwid-setFrequencies -p (frequencias no inicio da coleta)"
    likwid-setFrequencies -p 2>&1
    echo
    lscpu
} > "$OUT/sistema.txt"
likwid-topology -c -g > "$OUT/topologia.txt" 2>&1
log "Topologia salva em $OUT/topologia.txt (para a secao Arquitetura)"

# -----------------------------------------------------------------------------
# 2. Grupos do LIKWID (com as alternativas que o enunciado permite)
# -----------------------------------------------------------------------------
GRUPOS_DISP=$(likwid-perfctr -a 2>/dev/null | awk '{print $1}')
tem() { echo "$GRUPOS_DISP" | grep -qx "$1"; }

G_MEM=MEM;       tem MEM     || G_MEM=L3
G_CACHE=L2CACHE; tem L2CACHE || { if tem L1CACHE; then G_CACHE=L1CACHE; else G_CACHE=CACHE; fi; }
GRUPOS="$G_MEM $G_CACHE FLOPS_DP"
tem FLOPS_AVX && GRUPOS="$GRUPOS FLOPS_AVX"
echo "$GRUPOS" > "$OUT/grupos.txt"
log "Grupos que serao medidos: $GRUPOS"

# -----------------------------------------------------------------------------
# 3. Compilacao: uma versao SEM LIKWID (para medir tempo, sem interferencia
#    dos marcadores) e outra COM LIKWID (para as metricas)
# -----------------------------------------------------------------------------
log "Compilando versao para tempo (sem LIKWID)..."
make clean >/dev/null
make > "$OUT/compilacao.log" 2>&1 || { tail -20 "$OUT/compilacao.log"; erro "falha na compilacao"; }
cp broyden bin/otimizado_tempo
cp broyden-nao-otimizado bin/nao-otimizado_tempo

log "Compilando versao com marcadores LIKWID (LIKWID_HOME=${LIKWID_HOME:-/usr/local})..."
make clean >/dev/null
make LIKWID=1 ${LIKWID_HOME:+LIKWID_HOME="$LIKWID_HOME"} > "$OUT/compilacao_likwid.log" 2>&1 || { tail -20 "$OUT/compilacao_likwid.log"; erro "falha na compilacao com LIKWID (biblioteca nao encontrada?)"; }
cp broyden bin/otimizado_likwid
cp broyden-nao-otimizado bin/nao-otimizado_likwid
make clean >/dev/null

# -----------------------------------------------------------------------------
# 4. Testes de sanidade antes da coleta
# -----------------------------------------------------------------------------
ENT="32 $X0 $EPS $MAXIT"
if echo " $VERSOES " | grep -q " otimizado "; then
    log "Conferindo se as duas versoes dao o mesmo resultado (N=32)..."
    echo "$ENT" | bin/otimizado_tempo     | grep '^x' | awk '{print $3}' > "$OUT/v_otim.tmp"
    echo "$ENT" | bin/nao-otimizado_tempo | grep '^x' | awk '{print $3}' > "$OUT/v_nao.tmp"
    DIF=$(paste "$OUT/v_otim.tmp" "$OUT/v_nao.tmp" | awk 'BEGIN{m=0; n=0} {d=$1-$2; if(d<0)d=-d; if(d>m)m=d; n++} END{if(n==0) print "vazio"; else print m}')
    rm -f "$OUT"/v_*.tmp
    if [ "$DIF" = "vazio" ] || awk -v d="$DIF" 'BEGIN{exit !(d > 1e-8)}'; then
        erro "a versao otimizada nao bate com a nao otimizada (diferenca: $DIF). O solver do Arthur (gaussTridiagonal) ja foi implementado?"
    fi
    log "OK (maior diferenca: $DIF)"
else
    log "Versao otimizada nao sera medida: comparacao entre versoes pulada"
fi

PRIMEIRA="${VERSOES%% *}"
log "Conferindo se os marcadores LIKWID funcionam..."
echo "$ENT" | likwid-perfctr -C "$CORE" -g FLOPS_DP -m "bin/${PRIMEIRA}_likwid" > "$OUT/teste_marcadores.txt" 2>&1
for R in newton jacobiana sl; do
    grep -qE "Region:? +$R\b" "$OUT/teste_marcadores.txt" || {
        tail -15 "$OUT/teste_marcadores.txt"
        erro "a regiao '$R' nao apareceu na saida do LIKWID (tente rodar com sudo)"
    }
done
# Regioes aparecendo nao basta: sem acesso aos contadores (ex.: na macalan)
# o LIKWID imprime as tabelas com tudo zerado. Exige ciclos > 0.
if grep -q 'MSR write operation failed' "$OUT/teste_marcadores.txt" || \
   ! awk -F'|' '/CPU_CLK_UNHALTED_CORE/ {v=$4; gsub(/ /,"",v); if (v+0 > 0) ok=1} END {exit !ok}' "$OUT/teste_marcadores.txt"; then
    grep -m5 -E 'ERROR|MSR|CPU_CLK_UNHALTED_CORE' "$OUT/teste_marcadores.txt"
    erro "os contadores do LIKWID vieram zerados (sem acesso aos MSR). Use uma maquina do Lab-12 (hXX), nao a macalan."
fi
log "OK (regioes newton, jacobiana e sl encontradas, contadores com valores)"

# -----------------------------------------------------------------------------
# 5. Coleta. Ordem: versao otimizada primeiro (rapida), depois a nao otimizada
#    em N crescente. Se precisar interromper, os dados ja coletados ficam salvos.
# -----------------------------------------------------------------------------
echo "versao,N,rep,total_ms,jacobiana_ms,sl_ms" > "$OUT/tempos.csv"

for V in $VERSOES; do
    for N in $NS; do
        if [ "$V" = "nao-otimizado" ] && [ "$N" -gt "$NMAX_NAO_OTIM" ]; then
            log "$V N=$N: pulado (maior que NMAX_NAO_OTIM=$NMAX_NAO_OTIM)"
            continue
        fi
        ENT="$N $X0 $EPS $MAXIT"

        # Tempo (sem LIKWID), REPS vezes, preso ao nucleo CORE
        for R in $(seq 1 "$REPS"); do
            log "$V N=$N tempo (rep $R/$REPS)"
            T=$(echo "$ENT" | taskset -c "$CORE" "bin/${V}_tempo" | tail -4 | head -3 | paste -sd,)
            echo "$V,$N,$R,$T" >> "$OUT/tempos.csv"
        done

        # Metricas LIKWID (as linhas "x1 = ..." e "#" do programa sao descartadas)
        for G in $GRUPOS; do
            log "$V N=$N LIKWID $G"
            echo "$ENT" | likwid-perfctr -C "$CORE" -g "$G" -m "bin/${V}_likwid" 2>&1 \
                | grep -vE '^(x[0-9]+ = |#+$)' > "$OUT/raw/${V}_${G}_${N}.txt"
        done
    done
done

# -----------------------------------------------------------------------------
# 6. Empacota tudo para levar ao Windows
# -----------------------------------------------------------------------------
rm -rf bin
tar czf "${OUT}_likwid.tar.gz" "$OUT"
echo
log "PRONTO! Leve para o Windows o arquivo: $(pwd)/${OUT}_likwid.tar.gz"
