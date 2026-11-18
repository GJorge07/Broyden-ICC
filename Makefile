# Trabalho de ICC - SNLB com Metodo de Newton
#   make            -> gera ./broyden (otimizado) e ./broyden-nao-otimizado
#   make LIKWID=1   -> o mesmo, instrumentado com marcadores LIKWID (Linux)
#   make clean      -> remove objetos e executaveis

CC      = gcc
CFLAGS  = -O3 -march=native -mavx -fopt-info-vec -Wall
LDLIBS  = -lm

ifdef LIKWID
  LIKWID_HOME ?= /usr/local
  CFLAGS += -DLIKWID_PERFMON -I$(LIKWID_HOME)/include
  LDLIBS += -L$(LIKWID_HOME)/lib -llikwid
endif

HEADERS = $(wildcard *.h)
COMUNS  = main.o utils.o broyden.o jacobiana.o sistemas_lineares.o

all: broyden broyden-nao-otimizado

broyden: $(COMUNS) newton.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

broyden-nao-otimizado: $(COMUNS) newton_nao_otimizado.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Compila cada modulo individualmente
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o broyden broyden-nao-otimizado broyden.exe broyden-nao-otimizado.exe

.PHONY: all clean
