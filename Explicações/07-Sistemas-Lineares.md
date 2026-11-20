# 07. `sistemas_lineares.c`: Gauss denso e Gauss tridiagonal

## Versão não otimizada: `gaussDenso(A, b, n)`

Eliminação de Gauss com **pivoteamento parcial** + retrossubstituição. Resolve A·x = b
"in-place": A é destruída e a solução volta em b. Retorna −1 se a matriz for singular.

```
para cada coluna k:
    p = linha >= k com maior |A[p][k]|          (pivoteamento parcial)
    se |A[p][k]| == 0: singular, retorna -1
    troca as linhas k e p (só da coluna k em diante; à esquerda já é tudo zero)
    para cada linha i > k:
        m = A[i][k] / A[k][k]
        A[i][j] -= m * A[k][j]   para j > k     <- laço interno, vetorizado com AVX
        b[i]    -= m * b[k]
retrossubstituição: x[i] = (b[i] - Σ A[i][j]·x[j]) / A[i][i], de baixo para cima
```

- **Pivoteamento parcial:** escolhe como pivô o maior elemento da coluna, evitando dividir por
  zero ou por números pequenos (que amplificam o erro de arredondamento).
- **Propositalmente não otimizado:** opera sobre todos os n² elementos, inclusive os zeros da
  matriz tridiagonal. Custo ≈ ⅔n³ flops por chamada. N = 4000 → ~43 GFLOP por iteração.
- **O laço interno vetoriza** (`sistemas_lineares.c:41` e `:50`): `A[i][j] -= m*A[k][j]` com j
  variando é acesso sequencial, sem dependência entre j's. Por isso o FLOPS_AVX dele é alto.
- **Neste problema nunca troca linhas:** a Jacobiana é diagonalmente dominante, então o pivô
  já é o elemento da diagonal.

## Versão otimizada: `gaussTridiagonal(sub, diag, sup, b, n)`, feita pelo Arthur

Linha i do sistema: `sub[i]·x[i−1] + diag[i]·x[i] + sup[i]·x[i+1] = b[i]`.

```c
/* Triangularizacao */
for (int i = 0; i < n - 1; ++i) {
    double inv = 1.0 / diag[i];
    diag[i] = inv;                     // guarda o inverso para a volta
    double m = sub[i + 1] * inv;       // multiplicador
    diag[i + 1] -= m * sup[i];
    b[i + 1]    -= m * b[i];
}

/* Retrossubstituicao no proprio b */
b[n - 1] /= diag[n - 1];
for (int i = n - 2; i >= 0; --i)
    b[i] = (b[i] - sup[i] * b[i + 1]) * diag[i];
```

### Como funciona

1. **Ida (triangularização), i = 0 … n−2.** Para zerar `sub[i+1]` (o elemento abaixo da
   diagonal na linha i+1): linha(i+1) −= m · linha(i), com m = sub[i+1]/diag[i]. A linha i só
   tem `diag[i]` e `sup[i]` dali para a frente, então só **dois números** mudam: `diag[i+1]`
   e `b[i+1]`. O `sub[i+1]` viraria zero, mas nem é escrito, porque nunca mais é lido.
2. **Volta (retrossubstituição), i = n−1 … 0.** A matriz virou bidiagonal superior:
   x[n−1] = b[n−1]/diag[n−1] e x[i] = (b[i] − sup[i]·x[i+1]) / diag[i]. A solução é escrita
   no próprio `b`.

### Truque do inverso

Na ida, `1/diag[i]` já é calculado para o multiplicador; ele é **guardado no lugar de
`diag[i]`**. Na volta, em vez de dividir, multiplica pelo inverso. Divisão é bem mais cara que
multiplicação (latência ~4–5× maior), então troca n−1 divisões por multiplicações. Só a
última continua divisão (`diag[n−1]` não passa pelo laço da ida).

### Contrato com o `newton.c`

- Escreve **só** em `diag` e `b`. `sub` e `sup` são `const` e ficam intactos, porque o
  `newton.c` calcula os dois uma única vez e reaproveita nas 25 iterações.
- `restrict` nos quatro ponteiros.
- `diag` é recalculado pelo `jacobianaTri` a cada iteração, então pode ser destruído.

### Custo

O(n): dois laços de n passos, ≈ 9n flops por chamada (6 por linha na ida, 3 na volta;
confirmado pelo LIKWID: 9,0 flops escalares por linha), contra ≈ ⅔n³ do Gauss denso. Em
N = 4000: ~36 mil flops contra ~43 bilhões.

### Por que sem pivoteamento

1. Trocar linhas tiraria a matriz do formato tridiagonal (apareceria um elemento na segunda
   superdiagonal), e o algoritmo deixaria de ser O(n) com 3 vetores.
2. Não é necessário: a Jacobiana é **diagonalmente dominante** (|−4x_i+3| > 3, ver 02).
3. Prova prática: o Gauss denso, **com** pivoteamento, nunca troca linhas neste problema, e
   as duas versões dão o mesmo resultado (diferença ≤ 1e−15).

### Por que não vetoriza (e não tem como)

`-fopt-info-vec-all` mostra "couldn't vectorize loop" nas linhas 65 e 75. Cada passo depende
do anterior:
- na ida, `diag[i+1]` precisa do `diag[i]` recém-atualizado;
- na volta, `b[i]` precisa do `b[i+1]` recém-calculado.

É uma **recorrência** (dependência entre iterações, slide 18 da Parte 9), e não dá para
calcular 4 linhas de uma vez com AVX. Consequência nos dados: **FLOPS_AVX da região `sl` da
versão otimizada = 0** em todos os N. Não é erro.

Também não adianta desenrolar (unroll): a dependência continua.

### Por que a saída não é idêntica bit a bit

`a·(1/b)` e `a/b` podem diferir no último bit, porque em ponto flutuante as duas contas
arredondam de forma diferente. Diferença máxima medida: **9,99·10⁻¹⁶**. O script de coleta
aceita até 1e−8.

### Histórico

O Arthur entregou no commit `5ebccaf` (19/11) separando o código em `src_ingenuo/` e
`src_otimizado/`. Como o Makefile da raiz já gera as duas versões e o script de coleta depende
dessa estrutura, o código voltou para a raiz no commit `67247ad`, **sem nenhuma alteração** no
conteúdo (conferido pelo hash dos arquivos no git).
