# Etapa B: instruções para o Claude Code no Linux

> Leia este arquivo **inteiro** antes de rodar qualquer coisa. Ele substitui as instruções de
> etapas de `PARA_O_CLAUDE.md`; aquele arquivo e o `NOTAS_ETAPA_A.md` servem só de consulta.

## Situação atual (19/11/2026)

| Item | Estado |
|---|---|
| Etapa A (versão **não otimizada**) | ✅ Feita na `h52`, em 18/11. Resultado no repositório: `resultados_coleta/resultados_nao-otimizado_likwid.tar.gz`. **Não refazer.** |
| Solver otimizado (`gaussTridiagonal`, do Arthur) | ✅ Implementado (commit `5ebccaf`) e conferido no Windows: diferença máxima de 9,99e-16 para a versão não otimizada |
| Estrutura de pastas | O Arthur tinha separado o código em `src_ingenuo/` e `src_otimizado/`. No commit `67247ad` o código voltou para a **raiz**, sem alterar nada. Se essas pastas reaparecerem depois do `git pull`, **pare e avise o Gabriel** |
| **Etapa B (versão otimizada)** | ⏳ **É o que falta, e é só isso que você vai fazer** |

## Objetivo

Medir a versão **otimizada** (`./broyden`) com o LIKWID, **na mesma máquina e nas mesmas
condições da Etapa A**, e devolver o resultado pelo GitHub. Não é para analisar nem para
gerar gráficos: isso é feito no Windows.

## Onde rodar

- Máquina **`h52`** do Lab-12/DINF (Intel i5-7500), a mesma da Etapa A. O enunciado exige
  "mesma máquina" e "igualdade de condições", então **não use outra máquina**. Se a `h52`
  estiver fora do ar ou ocupada, pare e avise o Gabriel.
- Acesso: `ssh <login>@macalan.c3sl.ufpr.br` e depois `ssh h52`. A `macalan` é só passagem:
  ela zera os contadores e o enunciado proíbe servidoras compartilhadas.
- Antes de começar, confira com `who` e `uptime` se tem mais alguém usando a `h52`. Se a carga
  estiver alta, avise o Gabriel antes de medir.

## Passo a passo

```bash
# 1. Atualizar o código
cd ~/Broyden-ICC
git pull
git log --oneline -3                          # o topo deve ter 67247ad ou um commit mais novo
ls                                            # Makefile e os .c na raiz, SEM src_ingenuo/src_otimizado
grep -n "TODO(Arthur)" sistemas_lineares.c    # NÃO deve aparecer nada

# 2. Teste rápido (~1 min): compila, compara as versões em N=32 e confere os contadores
VERSOES=otimizado ./likwid/roda_testes.sh teste
#    tem que terminar com "PRONTO!"

# 3. Coleta completa, dentro do tmux (se a conexão cair, a coleta continua)
tmux new -s coletaB
VERSOES=otimizado ./likwid/roda_testes.sh 2>&1 | tee coleta_B.log
#    para sair do tmux sem parar: Ctrl+B e depois D; para voltar: tmux attach -t coletaB
```

**Duração esperada: poucos minutos.** São 12 valores de N × (3 medições de tempo + 4 grupos).
No Windows, o N=20000 inteiro roda em ~9 ms. Se passar de 30 minutos, tem algo errado: pare e
investigue.

O script já faz sozinho:
- CORE 3;
- `likwid-setFrequencies -g performance -c 3` no início e `powersave` no final;
- a compilação com as flags do enunciado (`-O3 -march=native -mavx -fopt-info-vec`);
- os grupos MEM, L2CACHE, FLOPS_DP e FLOPS_AVX;
- os N de 32 a 20000;
- x0 = −1, ε = 0, MAX = 25.

Saída: pasta `resultados_otimizado/` e arquivo `resultados_otimizado_likwid.tar.gz`.

## Conferências obrigatórias depois da coleta

Faça todas. Se alguma falhar, **não envie**: relate ao Gabriel.

1. **Mesmas condições da Etapa A.** Compare o `resultados_otimizado/sistema.txt` com o da Etapa A:
   ```bash
   mkdir -p /tmp/etA && tar xzf resultados_coleta/resultados_nao-otimizado_likwid.tar.gz -C /tmp/etA
   diff <(sed -n '2,12p' /tmp/etA/resultados_nao-otimizado/sistema.txt) \
        <(sed -n '2,12p' resultados_otimizado/sistema.txt)
   ```
   - Devem bater: máquina `h52`, kernel, `gcc (Debian 14.2.0-19)`, LIKWID 5.5.2, Governor
     `performance`, núcleo 3, `no_turbo` = 0 e os parâmetros.
   - **Diferença esperada e aceitável:** a linha `Flags`. A Etapa A usou `-mavx2` por engano; a B
     usa `-mavx`. No i5-7500 o código gerado é idêntico (ver `NOTAS_ETAPA_A.md`, seção 2).
   - Se o kernel, o gcc ou o LIKWID tiverem mudado, anote isso nas notas (passo 5).
2. **`tempos.csv` completo:** 1 linha de cabeçalho + 36 linhas (12 N × 3 repetições), todas com
   3 tempos numéricos e nenhum campo vazio.
   ```bash
   wc -l resultados_otimizado/tempos.csv                     # 37
   awk -F, 'NR>1 && NF!=6' resultados_otimizado/tempos.csv   # não deve imprimir nada
   ```
3. **48 arquivos em `raw/`** (12 N × 4 grupos), cada um com as 3 regiões:
   ```bash
   ls resultados_otimizado/raw | wc -l                       # 48
   for f in resultados_otimizado/raw/*.txt; do
     n=$(grep -cE 'Region (newton|jacobiana|sl),' "$f"); [ "$n" -ge 3 ] || echo "FALTA REGIAO: $f"
   done
   ```
4. **Estouro do contador MEM** (ver `NOTAS_ETAPA_A.md`, seção 5). Em cada
   `raw/otimizado_MEM_<N>.txt`, o `Memory data volume [GBytes]` da região `newton` tem que ser
   ≥ o da `sl` + o da `jacobiana`. A versão otimizada trafega pouco dado, então não deve
   estourar, mas confira, principalmente em N=20000.

### O que é normal e NÃO é erro

- **FLOPS_AVX da região `sl` perto de zero.** O Gauss tridiagonal tem dependência entre
  iterações (cada linha usa o resultado da anterior), então não vetoriza. O
  `compilacao.log` mostra isso para `sistemas_lineares.c` nas linhas ~65 e ~75.
- **Tempos minúsculos e variação grande entre repetições em N pequeno**, na casa dos
  microssegundos.
- **Banda de memória baixa e ruidosa para N pequeno**: os dados cabem na cache e o contador
  MEM é do socket inteiro.
- **MFLOP/s da `newton` em N pequeno bem abaixo da `sl`**: o custo dos próprios marcadores
  LIKWID dentro da região pesa quando o trabalho é pequeno.

## 5. Notas da Etapa B

Crie `likwid/NOTAS_ETAPA_B.md`, curto, no mesmo estilo do `NOTAS_ETAPA_A.md`:
- data, máquina e duração da coleta;
- resultado de cada conferência acima;
- tabela de tempos médios (média das 3 repetições) por N: total, jacobiana e sl;
- qualquer coisa diferente do esperado, como estouro do MEM, versão de software diferente ou
  máquina com carga.

## 6. Enviar para o GitHub

O `.gitignore` ignora `*.tar.gz` e `resultados*/`, por isso o `-f`:
```bash
mv resultados_otimizado_likwid.tar.gz resultados_coleta/
git add -f resultados_coleta/resultados_otimizado_likwid.tar.gz
git add likwid/NOTAS_ETAPA_B.md
git commit -m "Resultados da Etapa B (versao otimizada, h52)"
git push
```
Envie **só** esses dois arquivos. Não comite `coleta_B.log`, `resultados_otimizado/` nem
binários.

## Até onde ir: pare depois do push

Depois do push, avise o Gabriel que a Etapa B terminou e resuma as conferências. **Fim do
trabalho no Linux.**

## O que NÃO fazer

- Não refazer a Etapa A nem rodar a versão não otimizada. Use sempre `VERSOES=otimizado`.
- Não alterar código, Makefile, flags, parâmetros ou o script de coleta. Se algo exigir mudança,
  pare e pergunte ao Gabriel.
- Não mexer no solver do Arthur (`gaussTridiagonal`). Se a comparação entre versões falhar,
  relate o erro; não corrija.
- Não gerar gráficos, não processar dados além da tabela de tempos das notas e não escrever
  o relatório.
- Não rodar em outra máquina além da `h52`.
- Não fazer `git push --force` nem reescrever o histórico.

## Problemas prováveis

| Problema | Ação |
|---|---|
| `a versao otimizada nao bate com a nao otimizada` | Não medir. Conferir se o `git pull` trouxe o commit `67247ad` e se o `gaussTridiagonal` tem código. Relatar ao Gabriel. |
| `contadores do LIKWID vieram zerados` | Você está na `macalan`, não na `h52`. Dar `ssh h52`. |
| Erro de compilação com `likwid.h` | `LIKWID_HOME=/home/soft/likwid VERSOES=otimizado ./likwid/roda_testes.sh` |
| `src_ingenuo/` ou `src_otimizado/` reapareceram | Parar e avisar o Gabriel (o Arthur pode ter enviado algo novo). |
| `git push` rejeitado (o remoto tem commits novos) | `git pull --rebase` e push de novo. Se der conflito, parar e avisar. |
