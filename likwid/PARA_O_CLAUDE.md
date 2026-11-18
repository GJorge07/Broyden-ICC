# Instruções para o Claude Code (sessão no Linux)

> **ATUALIZAÇÃO (18/11/2026), leia antes do resto:**
> - A coleta é feita no **Lab-12/DINF, máquina `h52`** (Intel i5-7500), via
>   `ssh <login>@macalan.c3sl.ufpr.br` e depois `ssh h52`. **Etapa B na mesma `h52`.**
> - O notebook do Gabriel (Core Ultra 5 125H, Meteor Lake) não é suportado pelo LIKWID;
>   a `macalan` não dá acesso aos MSR (contadores zerados). Não usar nenhum dos dois.
> - Enunciado: compilar com `-O3 -mavx2 -march=native`; usar SEMPRE o CORE 3; fixar a
>   frequência com `likwid-setFrequencies -g performance -c 3` antes e
>   `likwid-setFrequencies -g powersave -c 3` depois. O script já faz tudo isso
>   (CORE padrão = 3) e funciona sem sudo.
> - O script aborta se os contadores vierem zerados.
> - Etapa B: `git pull` na `h52` e `VERSOES=otimizado ./likwid/roda_testes.sh` dentro de `tmux`.

Você está ajudando o Gabriel a rodar a **coleta de dados de desempenho com LIKWID** de um
trabalho da disciplina CI1164 (UFPR). O trabalho foi desenvolvido no Windows com outra sessão
do Claude; no Linux o objetivo é **só a coleta**. Siga este documento; não reescreva o código
nem o script sem necessidade.

## Contexto do projeto (resumo)

- Programa `broyden`: resolve o Sistema Não-Linear de Broyden pelo Método de Newton.
  Entrada (stdin): `N x0 epsilon MAX`. Saída: as iterações (`x1 = ...`, separadas por `#`) e,
  no final, entre linhas `###########`, 3 tempos em ms: total do Newton, soma das Jacobianas,
  soma dos sistemas lineares.
- Duas versões, mesma `main.c`:
  - `broyden-nao-otimizado` (`newton_nao_otimizado.c`): Jacobiana densa n×n + Gauss denso O(n³);
  - `broyden` (`newton.c`): Jacobiana tridiagonal em 3 vetores + Gauss tridiagonal O(n)
    (`gaussTridiagonal` em `sistemas_lineares.c`, implementado pelo Arthur).
- Marcadores LIKWID já estão no código: regiões **`newton`**, **`jacobiana`**, **`sl`**.
  O header `likwid_markers.h` usa a API real só com `-DLIKWID_PERFMON` (`make LIKWID=1`).
- Flags obrigatórias do enunciado (já no Makefile): `-O3 -mavx2 -march=native` (+ `-fopt-info-vec`, so relatorio de vetorizacao).
- Parâmetros obrigatórios dos testes: `MAX=25`, `epsilon=0`.
  `x0=-1` (com 0.5 o Newton não converge; com eps=0 são sempre 25 iterações de qualquer forma).
- N exigidos: 32 64 128 256 512 1000 2000 4000 8000 9000 10000 20000.
  A versão não otimizada só vai até `NMAX_NAO_OTIM` (padrão 4000): acima disso a matriz
  densa é enorme (N=20000 → 3,2 GB) e leva horas. Isso será justificado no relatório.
- Grupos LIKWID exigidos: `MEM` (ou `L3`), `L2CACHE` (ou `L1CACHE`/`CACHE`), `FLOPS_DP`,
  `FLOPS_AVX`. O script escolhe as alternativas automaticamente.

## Coleta em duas etapas

O solver otimizado (`gaussTridiagonal`, do Arthur) pode ainda não estar pronto. Nesse caso:
- **Etapa A (não depende dele):** `VERSOES=nao-otimizado ./likwid/roda_testes.sh` — mede só a
  versão não otimizada (a parte demorada, ~1 h) e salva a topologia. Saída:
  `resultados_nao-otimizado/` e `resultados_nao-otimizado_likwid.tar.gz`.
- **Etapa B (depois do solver):** `git pull` e `VERSOES=otimizado ./likwid/roda_testes.sh`
  (minutos). Saída: `resultados_otimizado/` e o `.tar.gz` correspondente.
Sem `VERSOES`, o script mede as duas e exige o solver implementado.
As duas etapas devem rodar na mesma máquina e nas mesmas condições (na tomada, nada aberto).
Para saber se o solver já existe: `grep -n "TODO(Arthur)" sistemas_lineares.c` (se aparecer,
ainda não foi implementado → só a etapa A).

## O que fazer

1. Conferir que o LIKWID está instalado (`likwid-perfctr -a`). Se não estiver:
   `sudo apt install -y likwid build-essential linux-tools-common linux-tools-$(uname -r)`.
2. Na raiz do projeto: `chmod +x likwid/roda_testes.sh` e rodar o teste rápido
   `./likwid/roda_testes.sh teste`. Deve terminar com `PRONTO!`.
   - O script pede `sudo` (modprobe msr, perf_event_paranoid, governor `performance`).
     Peça ao Gabriel para digitar a senha; não tente contornar.
3. Se o teste passar, orientar o Gabriel a: ligar o notebook na tomada, fechar outros
   programas e rodar a coleta completa `./likwid/roda_testes.sh` (~1 h). Rodar em primeiro
   plano no terminal dele, ou em background acompanhando o log.
4. Ao final, confirmar que existe o `resultados*_likwid.tar.gz` e que o `tempos.csv` e os
   `raw/*.txt` da pasta `resultados*/` não estão vazios (cada `raw` deve conter
   linhas `Region newton`, `Region jacobiana`, `Region sl` com métricas).
5. Ajudar a copiar o(s) `resultados*_likwid.tar.gz` para a partição do Windows, para a pasta
   `C:\Users\Gabriel Jorge\Projetos\Trabalho ICC\` (no Linux, o disco do Windows costuma
   aparecer em `/media/<usuario>/...` depois de aberto no gerenciador de arquivos).

## O que o script faz (`likwid/roda_testes.sh`)

1. Carrega `msr`, ajusta `perf_event_paranoid`, fixa governor `performance`.
2. Salva `resultados/sistema.txt` (gcc, lscpu, governor) e `resultados/topologia.txt`
   (`likwid-topology -c -g`, exigido para a seção Arquitetura do relatório).
3. Compila duas vezes: **sem** LIKWID (para medir tempo sem interferência dos marcadores)
   e **com** LIKWID (`make LIKWID=1`). Guarda os logs de compilação (contêm a saída do
   `-fopt-info-vec`, útil para o relatório).
4. Sanidade: compara as saídas das duas versões para N=32 (tolerância 1e-8) — aborta se
   não baterem (indica solver do Arthur ausente/errado); confere se as regiões
   `newton/jacobiana/sl` aparecem no `likwid-perfctr -m`.
5. Para cada versão e N: `REPS` medições de tempo com `taskset -c CORE`
   (→ `resultados/tempos.csv`) e uma execução `likwid-perfctr -C CORE -g GRUPO -m` por grupo
   (→ `resultados/raw/<versao>_<grupo>_<N>.txt`, com as linhas `x = ...` filtradas).
6. Empacota em `resultados_likwid.tar.gz`.

## Problemas prováveis e o que fazer

| Problema | Ação |
|---|---|
| `a versao otimizada nao bate...` | Não medir. Verificar se `gaussTridiagonal` em `sistemas_lineares.c` está implementado (não pode ser só o `TODO(Arthur)`). Não implementar no lugar do Arthur sem o Gabriel pedir explicitamente. |
| Regiões não aparecem no LIKWID | Rodar com `sudo`; conferir `lsmod \| grep msr`; conferir se a compilação usou `-DLIKWID_PERFMON` (ver `resultados*/compilacao_likwid.log`). |
| `likwid.h` / `-llikwid` não encontrados | Localizar com `dpkg -L likwid` e rodar com `LIKWID_HOME=<prefixo> ./likwid/roda_testes.sh` (o Makefile usa `$(LIKWID_HOME)/include` e `$(LIKWID_HOME)/lib`). |
| Grupo inexistente na CPU | O script já troca `MEM→L3` e `L2CACHE→L1CACHE/CACHE`; se `FLOPS_DP` não existir, avisar o Gabriel (o enunciado exige). |
| Coleta lenta demais | `NMAX_NAO_OTIM=2000 ./likwid/roda_testes.sh` e avisar o Gabriel do corte. |
| Script com `\r` (CRLF) | `sed -i 's/\r$//' likwid/roda_testes.sh` |

## O que NÃO fazer

- Não alterar as flags de compilação nem os parâmetros (MAX=25, eps=0) — são exigidos.
- Não otimizar nem reescrever o código: o objetivo aqui é só medir.
- Não rodar a coleta com outros programas pesados abertos.
- Não processar os dados nem gerar gráficos aqui: isso será feito no Windows.
