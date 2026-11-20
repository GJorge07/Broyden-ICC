# 22. Perguntas prováveis e estudo para a prova (04/12)

## Perguntas sobre o trabalho

**Por que a Jacobiana é tridiagonal?**
Cada f_i só depende de x_{i−1}, x_i e x_{i+1}; as outras derivadas parciais são zero.

**Por que só a diagonal é recalculada a cada iteração?**
∂f_i/∂x_{i−1} = −1 e ∂f_i/∂x_{i+1} = −2 não dependem de x. Só ∂f_i/∂x_i = −4x_i + 3 muda.

**Qual a complexidade de cada versão por iteração?**
Não otimizada: Jacobiana O(n²) + Gauss O(n³). Otimizada: Jacobiana O(n) + Gauss tridiagonal O(n).

**Por que o Gauss tridiagonal não usa pivoteamento?**
Trocar linhas criaria elementos fora das três diagonais. A Jacobiana é diagonalmente dominante,
então não é necessário; o Gauss denso, com pivoteamento, nunca troca linhas neste problema.

**Por que o Gauss tridiagonal não vetoriza, e a Jacobiana sim?**
No Gauss, a linha i depende do resultado da linha i−1 (recorrência). Na Jacobiana, cada
posição é independente.

**Por que guardar `1/diag[i]`?**
Para trocar as divisões da volta por multiplicações, que são bem mais baratas. O inverso já é
calculado na ida.

**O que é fusão de laços e qual foi a desvantagem aqui?**
Juntar laços que percorrem os mesmos vetores: cada dado é lido uma vez e há menos controle de
laço. Desvantagem: o laço fundido tem uma redução de máximo, que não vetoriza sem
`-ffast-math`.

**O que é loop-invariant code motion?**
Tirar de dentro do laço um cálculo cujo resultado não muda: as diagonais −1 e −2.

**Para que serve o `restrict`?**
Promete que os ponteiros não se sobrepõem. Sem isso, o compilador precisa gerar código que
funcione com sobreposição (ou duas versões do laço e um teste em tempo de execução).

**Por que ε = 0 nos testes?**
Para todas as execuções fazerem exatamente 25 iterações: mesmo trabalho, comparação justa.

**Por que a impressão fica fora do Newton?**
Para tempo e métricas medirem só o cálculo, e não 520 mil `printf`.

**Como você sabe que a versão otimizada está certa?**
As duas versões coincidem até ~1e−15 (só arredondamento), e o resultado bate com uma
implementação independente em Python.

**Por que as versões não dão o mesmo resultado bit a bit?**
A ordem das operações mudou (fatoração, multiplicar pelo inverso), e em ponto flutuante a
associatividade não vale.

**Por que a versão otimizada tem MFLOP/s menor e é mais rápida?**
MFLOP/s mede o ritmo das contas. A otimizada faz ~8n flops em vez de ~⅔n³; menos trabalho
vale mais que trabalho rápido.

**Por que a banda da não otimizada salta em N = 1000?**
A matriz (8 MB) deixa de caber na L3 (6 MB) e passa a vir da RAM a cada iteração.

**Por que o L2 miss ratio da não otimizada cai em N = 128?**
A matriz de 128 kB cabe na L2 (256 kB).

**O que o LIKWID mede e como você separou os trechos?**
Contadores de hardware; Marker API com as regiões `newton`, `jacobiana` e `sl`.

**Por que o volume de memória da região `newton` ficou menor que o da `sl` em N = 4000?**
Estouro do contador de 32 bits (volta a cada ~274,9 GB) dentro de uma única chamada da região.

## Estudo para a prova prática

A prova é sobre aplicar técnicas de otimização a um código e mostrar o efeito. Treinar à mão:

- [ ] Derivar a Jacobiana de um sistema não linear e reconhecer quando é k-diagonal.
- [ ] Reescrever `jacobianaTri` e o laço de `newton.c` **sem olhar** e conferir com o teste
      de 16.
- [ ] Gauss tridiagonal de memória (ida e volta).
- [ ] Parte 9: subexpressões, código invariante fora do laço, loop peeling, `restrict`,
      `inline`, alinhamento, por que a associatividade impede o compilador.
- [ ] Parte 10: **unroll & jam** e **blocking** em multiplicação matriz-vetor e matriz-matriz.
      Não estão no trabalho, mas são o tipo de exercício que cai.
- [ ] Parte 8: usar o `likwid-perfctr` com `-C`, `-g`, `-m`; ler FLOPS_DP, MEM e L2CACHE;
      fixar a frequência (`likwid-setFrequencies`).
- [ ] Ler um gráfico log-log e tirar a complexidade pela inclinação.
