# Passo a passo no Linux (coleta com LIKWID)

> **19/11/2026 — COLETA CONCLUÍDA (Etapas A e B).** No Windows, leia `PARA_O_CLAUDE_WINDOWS.md` na raiz.
> **ETAPA B:** a Etapa A já foi feita e o solver do Arthur está pronto.
> Para a Etapa B, siga **`likwid/ETAPA_B_PARA_O_CLAUDE.md`**.

> **ONDE A COLETA FOI FEITA (Etapa A, 18/11/2026): máquina `h52` do Lab-12/DINF, CORE 3.**
> A Etapa B **tem que** ser na mesma `h52` (enunciado: mesma máquina e igualdade de condições).
> O notebook do Gabriel (Intel Core Ultra 5 125H, Meteor Lake híbrido) **não é suportado pelo
> LIKWID** (nem a versão do GitHub), e a `macalan` não dá acesso aos contadores (tudo zerado).
>
> No DINF (sem sudo; o script já trata isso):
> ```bash
> ssh <login>@macalan.c3sl.ufpr.br
> ssh h52
> cd ~/Broyden-ICC && git pull
> tmux new -s coleta
> VERSOES=otimizado ./likwid/roda_testes.sh 2>&1 | tee coleta_B.log
> ```
> O script fixa a frequência com `likwid-setFrequencies -g performance -c 3` e volta para
> `powersave` no final, como pede o enunciado.

Tudo o que precisa ser feito no Linux é **rodar um script**. Compilação, testes de sanidade,
fixar frequência, topologia, medições e empacotamento são automáticos.

---

## Dá para fazer em duas etapas (sem esperar o Arthur)

| Etapa | Precisa do solver do Arthur? | Comando | Duração |
|---|---|---|---|
| **A** — versão não otimizada + topologia | ❌ Não | `VERSOES=nao-otimizado ./likwid/roda_testes.sh` | ~1 h |
| **B** — versão otimizada | ✅ Sim | `VERSOES=otimizado ./likwid/roda_testes.sh` | poucos minutos |

A etapa A é a demorada e pode ser feita **já**. A B, depois que o Arthur terminar, é rápida.
Cada etapa gera seu próprio arquivo (`resultados_nao-otimizado_likwid.tar.gz` e
`resultados_otimizado_likwid.tar.gz`), e um não apaga o outro.

**Condição para as duas valerem juntas (enunciado: "mesmos parâmetros e igualdade de
condições"):** mesma máquina, notebook na tomada, nada mais rodando — o script fixa a
frequência e o núcleo do mesmo jeito nas duas.

## Antes de ir para o Linux (no Windows)

- [ ] `utils.c` / `utils.h` do professor no lugar dos provisórios — **antes da etapa A**,
      porque eles são usados pela versão não otimizada também (se trocar depois, a etapa A
      teria que ser refeita).
- [ ] Commit + push no GitHub (é por lá que o código chega no Linux).
- [ ] Só para a etapa B: o Arthur implementou o `gaussTridiagonal` (o script **aborta** se a
      versão otimizada não bater com a não otimizada — isso protege contra medir um solver vazio).
      Depois do push dele, no Linux: `git pull` antes de rodar a etapa B.

---

## No Linux

### 1. Instalar o necessário (uma vez só)

```bash
sudo apt update
sudo apt install -y likwid build-essential git linux-tools-common linux-tools-$(uname -r)
```
(`linux-tools-*` traz o `cpupower`, que fixa a frequência da CPU. Se der erro nesse pacote,
pode ignorar: o script tenta o `likwid-setFrequencies` no lugar.)

### 2. Baixar o código

```bash
cd ~
git clone https://github.com/GJorge07/Broyden-ICC.git
cd Broyden-ICC
chmod +x likwid/roda_testes.sh
```

### 3. Teste rápido (~1–2 min)

```bash
./likwid/roda_testes.sh teste                           # com o solver do Arthur pronto
VERSOES=nao-otimizado ./likwid/roda_testes.sh teste     # sem o solver (etapa A)
```
Vai pedir a senha do `sudo`. Tem que terminar com **`PRONTO!`**. Se aparecer `ERRO:`, veja
"Problemas comuns" abaixo.

### 4. Coleta completa (~1 hora)

Antes: **notebook na tomada**, fechar navegador e outros programas, não usar o computador
durante a coleta (slide 23: "sistema EXCLUSIVO").

```bash
./likwid/roda_testes.sh                                 # as duas versões de uma vez
# ou, em duas etapas:
VERSOES=nao-otimizado ./likwid/roda_testes.sh           # etapa A (~1 h)
git pull && VERSOES=otimizado ./likwid/roda_testes.sh   # etapa B (depois do Arthur)
```

### 5. Levar o resultado para o Windows

O script gera um **`resultados*_likwid.tar.gz`** na pasta do projeto. Copie para
`C:\Users\Gabriel Jorge\Projetos\Trabalho ICC\` por um destes caminhos:
- **Mais fácil:** no gerenciador de arquivos do Linux, abrir o disco do Windows
  ("Outros locais") e colar o arquivo lá;
- ou pendrive / Google Drive.

No Windows, o resto (gráficos e relatório) é feito com o Claude daqui.

---

## Usando o Claude Code no Linux

Se for usar o Claude lá, abra o Claude na pasta `Broyden-ICC` e diga:

> Leia `likwid/PARA_O_CLAUDE.md` e me ajude a fazer a coleta.

---

## Problemas comuns

| Sintoma | Solução |
|---|---|
| `likwid-perfctr nao encontrado` | `sudo apt install likwid` |
| `bad interpreter` ou `^M` | `sed -i 's/\r$//' likwid/roda_testes.sh` |
| `Permission denied` ao rodar o script | `chmod +x likwid/roda_testes.sh` |
| `a versao otimizada nao bate com a nao otimizada` | O solver do Arthur ainda não foi implementado (ou tem bug). Não dá para medir antes disso. |
| `a regiao 'newton' nao apareceu` | Rodar com sudo: `sudo ./likwid/roda_testes.sh teste` |
| Erro de compilação com `likwid.h` ou `-llikwid` | Descobrir onde o LIKWID está (`dpkg -L likwid \| grep likwid.h`) e rodar com `LIKWID_HOME=<pasta acima de include> ./likwid/roda_testes.sh` |
| A coleta está demorando demais | Interromper (Ctrl+C) e rodar `NMAX_NAO_OTIM=2000 ./likwid/roda_testes.sh` (limita o N da versão não otimizada) |

## Parâmetros opcionais

```bash
NMAX_NAO_OTIM=4000   # maior N da versão não otimizada (padrão 4000; acima disso leva horas)
REPS=3               # repetições de cada medição de tempo (padrão 3)
CORE=3               # núcleo onde o programa roda (padrão 3, exigido no Lab-12/DINF)
```
Exemplo: `NMAX_NAO_OTIM=2000 REPS=5 ./likwid/roda_testes.sh`
