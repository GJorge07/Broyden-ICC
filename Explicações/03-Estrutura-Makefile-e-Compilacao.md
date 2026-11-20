# 03. Estrutura do código, Makefile e compilação

## Módulos

| Arquivo | Conteúdo | Usado por |
|---|---|---|
| `main.c` | só a `main()`: lê a entrada, chama `newton()`, imprime | as duas versões |
| `newton.h` | `struct TemposNewton` e a assinatura de `newton()` | as duas |
| `newton_nao_otimizado.c` | `newton()` da versão não otimizada | `broyden-nao-otimizado` |
| `newton.c` | `newton()` da versão otimizada | `broyden` |
| `broyden.c/h` | `avaliaF` e `normaMax` | as duas |
| `jacobiana.c/h` | `jacobianaDensa` (não otimizada), `jacobianaTriConst` e `jacobianaTri` (otimizada) | as duas |
| `sistemas_lineares.c/h` | `gaussDenso` (não otimizada) e `gaussTridiagonal` (otimizada, Arthur) | as duas |
| `utils.c/h` | `timestamp()` | as duas |
| `likwid_markers.h` | marcadores LIKWID que viram "nada" sem `-DLIKWID_PERFMON` | as duas |

### Por que duas implementações da mesma função `newton()`

`newton_nao_otimizado.c` e `newton.c` definem **a mesma função** com a mesma assinatura. O
Makefile liga um ou o outro. Assim a leitura, a impressão e a medição (`main.c`) são
**idênticas** nos dois executáveis: a única diferença entre eles é o que foi otimizado, o que
torna a comparação dos gráficos justa.

## Makefile

```make
CC      = gcc
CFLAGS  = -O3 -march=native -mavx -fopt-info-vec -Wall
LDLIBS  = -lm

ifdef LIKWID                       # make LIKWID=1
  LIKWID_HOME ?= /usr/local
  CFLAGS += -DLIKWID_PERFMON -I$(LIKWID_HOME)/include
  LDLIBS += -L$(LIKWID_HOME)/lib -Wl,-rpath,$(LIKWID_HOME)/lib -llikwid
endif

COMUNS  = main.o utils.o broyden.o jacobiana.o sistemas_lineares.o
all: broyden broyden-nao-otimizado
broyden:               $(COMUNS) newton.o
broyden-nao-otimizado: $(COMUNS) newton_nao_otimizado.o
%.o: %.c $(HEADERS)        # cada módulo compilado separadamente
clean: rm -f *.o broyden broyden-nao-otimizado *.exe
```

| Comando | Resultado |
|---|---|
| `make` | `broyden` (otimizado) e `broyden-nao-otimizado` |
| `make LIKWID=1` | os dois, com os marcadores LIKWID ativos (Linux) |
| `make clean` | apaga objetos e executáveis |

Detalhes:
- **`-Wall`** foi acrescentado às flags obrigatórias. Só liga avisos e não muda o código gerado.
- **`.o` dependem de todos os `.h`**: se um header muda, tudo recompila. Simples e seguro.
- **`-Wl,-rpath`** grava no executável o caminho da `liblikwid.so`. Sem isso, no DINF o
  programa não acha a biblioteca ao rodar.
- **`LIKWID_HOME`**: no DINF o LIKWID está em `/home/soft/likwid`. O script de coleta descobre
  isso sozinho a partir do `likwid-perfctr` em uso.

## As flags do enunciado

| Flag | O que faz |
|---|---|
| `-O3` | otimização máxima "segura" do GCC, inclui a vetorização automática |
| `-march=native` | gera código para o processador da máquina onde compila (liga todas as instruções que ele tem: no i5-7500, AVX, AVX2 e FMA) |
| `-mavx` | liga as instruções AVX (vetores de 256 bits = 4 doubles). Com `-march=native` num processador com AVX, é redundante |
| `-fopt-info-vec` | imprime quais laços foram vetorizados. Útil para justificar as otimizações |

### `-mavx` × `-mavx2`

A Etapa A foi compilada com `-mavx2` por engano (uma página geral do LIKWID cita essa flag). O
código de máquina é **idêntico** ao de `-mavx` no i5-7500:
- no Linux, depois da Etapa A, todos os `.c` foram compilados com `-march=skylake -mavx` e `-mavx2`
  (Kaby Lake tem o mesmo conjunto de instruções do Skylake) e comparou;
- no Windows, em 19/11, repeti a comparação com `objdump -d`: os 7 arquivos saíram iguais byte
  a byte.

Motivo: `-march=native` já liga AVX e AVX2; `-mavx` ou `-mavx2` só acrescentariam o que já
está ligado. Ver 21 sobre como registrar isso no relatório.

## Compilar e rodar no Windows (só para testes de corretude)

```bash
export PATH="/c/msys64/mingw64/bin:$PATH"   # GCC 15; sem isso o .exe sai com código 127
cd "/c/Users/Gabriel Jorge/Projetos/Trabalho ICC/broyden"
mingw32-make
echo "4 -1 1e-10 25" | ./broyden
```

Medições de desempenho **só valem na `h52`** (Linux, LIKWID). Os tempos do Windows servem
apenas para ter ordem de grandeza.
