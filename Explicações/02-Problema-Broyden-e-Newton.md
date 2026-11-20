# 02. O problema: sistema de Broyden e Método de Newton

## O sistema

Queremos x = (x₁, …, xₙ) tal que F(x) = 0, com:

```
f_1(x) = -2x_1² + 3x_1 - 2x_2 + 1
f_i(x) = -2x_i² + 3x_i - x_{i-1} - 2x_{i+1} + 1,   2 <= i <= n-1
f_n(x) = -2x_n² + 3x_n - x_{n-1}                    (sem o "+1", como no enunciado)
```

Cada equação i só envolve x_{i−1}, x_i e x_{i+1}. Essa é a propriedade que permite toda a
otimização: a Jacobiana fica **tridiagonal**.

## Método de Newton para sistemas

Ideia: perto de x, F se comporta como sua aproximação linear F(x + Δ) ≈ F(x) + J(x)·Δ.
Igualando a zero:

```
J(x) · Δ = -F(x)        (sistema linear n×n)
x  <-  x + Δ
```

Repete até parar. Cada iteração faz:

1. calcular F(x);
2. calcular a Jacobiana J(x) → **trecho medido "jacobiana"**;
3. resolver J·Δ = −F → **trecho medido "sl"**;
4. atualizar x.

### Critérios de parada (`newton.h`)

- ‖F(x)‖∞ < ε: já está na raiz (testado antes de calcular a Jacobiana);
- ‖Δ‖∞ < ε: o passo ficou desprezível (testado depois de atualizar x);
- MAX iterações.

Com **ε = 0**, `algo < 0` nunca é verdade para uma norma, então **sempre são feitas exatamente
MAX = 25 iterações**. É por isso que o enunciado fixa ε = 0: todas as execuções fazem o mesmo
trabalho e as versões ficam comparáveis.

## A Jacobiana

J[i][j] = ∂f_i/∂x_j. Derivando f_i = −2x_i² + 3x_i − x_{i−1} − 2x_{i+1} + 1:

| Derivada | Valor | Depende de x? |
|---|---|---|
| ∂f_i/∂x_i | −4x_i + 3 | **sim**, muda a cada iteração |
| ∂f_i/∂x_{i−1} | −1 | não, constante |
| ∂f_i/∂x_{i+1} | −2 | não, constante |
| qualquer outra | 0 | — |

f_1 não tem x_0 e f_n não tem x_{n+1}, mas os valores das derivadas existentes são os mesmos
(o "+1" de f_1 e a falta dele em f_n somem na derivada).

```
      | 3-4x1   -2                        |
      |  -1   3-4x2   -2                  |
J  =  |        -1   3-4x3   -2            |
      |               ...   ...   ...     |
      |                     -1   3-4xn    |
```

Consequências:
- só **3n − 2** dos n² elementos podem ser diferentes de zero;
- as duas diagonais de fora são **constantes**;
- só a diagonal principal precisa ser recalculada a cada iteração.

## Diagonal dominante

|−4x_i + 3| > |−1| + |−2| = 3 sempre que x_i < 0. Na solução (N = 1000), os x_i ficam entre
−0,71 e −0,15, então a diagonal fica entre 3,6 e 5,8; no chute x0 = −1 ela vale 7.
Matriz diagonalmente dominante → Gauss sem pivoteamento é estável (ver 07).

## Chute inicial x0

O enunciado não fixa x0. Testes:
- **x0 = 0,5**: o Newton **não converge**. Nesse ponto a diagonal da Jacobiana vale
  −4·0,5 + 3 = 1, e já o primeiro passo leva os valores a ordens de grandeza enormes (~10⁴ em
  N = 32, ~10¹⁴⁹ em N = 1000); o método não se recupera nas 25 iterações. Não é bug: o Newton só converge
  se o chute estiver perto o bastante da raiz.
- **x0 = −1**: converge em **5 iterações** (ε = 1e−10, N = 4 e N = 1000).

Nos experimentos usamos **x0 = −1**. Com ε = 0 o custo seria o mesmo de qualquer forma
(25 iterações), mas assim os números calculados fazem sentido.
