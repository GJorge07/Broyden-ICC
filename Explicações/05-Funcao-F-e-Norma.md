# 05. `broyden.c`: F(x) e norma do máximo

Usado pelas duas versões. A otimizada usa a `normaMax`, mas calcula −F com uma função própria
(`avaliaMenosF`, ver 09).

## `avaliaF(x, F, n)`

```c
F[0] = -2.0*x[0]*x[0] + 3.0*x[0] - 2.0*x[1] + 1.0;                 // f_1

for (int i = 1; i < n - 1; ++i)                                      // f_2 .. f_{n-1}
    F[i] = -2.0*x[i]*x[i] + 3.0*x[i] - x[i-1] - 2.0*x[i+1] + 1.0;

F[n-1] = -2.0*x[n-1]*x[n-1] + 3.0*x[n-1] - x[n-2];                 // f_n
```

**Loop peeling**: a primeira e a última equação são diferentes das outras (faltam vizinhos,
f_n não tem o "+1"). Elas foram tiradas do laço, e o corpo do laço fica **sem `if`**. Sem
desvios condicionais, o GCC vetoriza (`-fopt-info-vec`: "loop vectorized using 32 byte
vectors"). Slides 14–15 da Parte 9: "evitar desvios dentro de laços separando os casos".

Sem `restrict`, o GCC não sabe se `x` e `F` se sobrepõem. Ele gera duas versões do laço e
escolhe em tempo de execução ("loop versioned for vectorization because of possible
aliasing"). Funciona, mas tem um teste a mais. A versão otimizada (`avaliaMenosF`) usa
`restrict` e elimina isso.

## `normaMax(v, n)`

```c
double max = 0.0;
for (int i = 0; i < n; ++i) {
    double a = fabs(v[i]);
    if (a > max) max = a;
}
return max;
```

‖v‖∞ = max |v_i|. Usada nos critérios de parada: ‖F‖∞ < ε e ‖Δ‖∞ < ε.

**Não vetoriza**: é uma redução de máximo em `double`. O GCC só a vetoriza com
`-ffast-math`, proibido pelo enunciado (as flags são fixas). Isso volta na fusão de laços
(ver 09).
