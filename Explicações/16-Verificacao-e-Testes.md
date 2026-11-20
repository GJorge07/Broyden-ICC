# 16. Verificação: como se sabe que o programa está certo

| Teste | Resultado |
|---|---|
| Compilação com as flags obrigatórias (GCC 15 no Windows, GCC 14 na `h52`) | sem erros nem warnings (`-Wall`) |
| 1ª iteração calculada **à mão** (N = 4, x0 = 0,5) | Δ = [2; 1,5; 0; −0,5], igual ao programa |
| Implementação **independente em Python** (Newton + Gauss) | mesmos x até ~10 casas decimais |
| N = 4 e N = 1000, x0 = −1, ε = 1e−10 | converge em **5 iterações**, ‖F‖ ≈ 4·10⁻¹⁶ |
| ε = 0, MAX = 25 | sempre **25 iterações** (26 blocos impressos, contando o x0) |
| Otimizada × não otimizada, N = 4, 32, 500, 1000 (Windows) | diferença máxima **9,99·10⁻¹⁶** |
| Otimizada × não otimizada, N = 2, 3, 32, 100, 1000, 2000 (Linux) | mesma diferença máxima |
| Solver do Arthur no AddressSanitizer (Linux) | sem erros de memória |
| Teste de sanidade automático do script na `h52` (N = 32) | diferença 9,99e−16, aceito |

## Por que as versões dão (quase) o mesmo resultado

A Jacobiana é diagonalmente dominante, então o Gauss com pivoteamento nunca troca linhas. Sem
trocas, ele faz as mesmas contas que o Gauss tridiagonal; as operações extras do denso são
todas com zeros e não alteram o resultado.

A diferença de ~1e−15 (cerca de 4 unidades no último bit de um double perto de 1) vem de:
- `avaliaMenosF` fatorar x·(2x − 3) em vez de −2x² + 3x;
- o Arthur multiplicar pelo inverso de `diag` em vez de dividir.

Em ponto flutuante a ordem das operações muda o arredondamento (slide 20 da Parte 9).

## Com x0 = 0,5 as versões divergem

Com x0 = 0,5 o Newton não converge e oscila de forma caótica. Qualquer diferença de
arredondamento cresce a cada iteração, e as saídas das duas versões se afastam. Por isso a
comparação é feita com x0 = −1.

## Como repetir

```bash
export PATH="/c/msys64/mingw64/bin:$PATH"
mingw32-make
echo "1000 -1 0 25" | ./broyden              | sed '/^###/,$d' > a.txt
echo "1000 -1 0 25" | ./broyden-nao-otimizado | sed '/^###/,$d' > b.txt
paste a.txt b.txt | grep -v '#' | awk '{d=$3-$6; if(d<0)d=-d; if(d>m)m=d} END{print "maior diferenca:", m}'
```
O `sed` corta os tempos do final, que são diferentes por natureza.
