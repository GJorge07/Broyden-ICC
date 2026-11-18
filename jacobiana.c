#include "jacobiana.h"

/*Versao não-otimizada: preenche a Jacobiana densa n x n de F no ponto x.
 Acesso ao elemento (linha i, coluna j): J[i*n + j].
 O vetor J não tem inicializacao (malloc), com n*n posicoes.*/
void jacobianaDensa(const double *x, double *J, int n) {
    
    /* zera a matriz inteira */
    for (int i = 0; i < n; ++i) 
        for (int j = 0; j < n; ++j)
            J[i * n + j] = 0.0;

    /* preenche as tres diagonais com as derivadas parciais */
    for (int i = 0; i < n; ++i) {

        J[i * n + i] = -4.0 * x[i] + 3.0;   //d f_i / d x_i     

        if (i > 0)
            J[i * n + i - 1] = -1.0;        //d f_i / d x_{i-1} 

        if (i < n - 1)
            J[i * n + i + 1] = -2.0;        //d f_i / d x_{i+1}

    }
}

/*Versao otimizada: a Jacobiana e tridiagonal, entao so as tres diagonais
 sao guardadas, cada uma em um vetor de tamanho n:
   sub[i]  = d f_i / d x_{i-1} = -1        (sub[0] nao e usado)
   diag[i] = d f_i / d x_i     = -4x_i + 3
   sup[i]  = d f_i / d x_{i+1} = -2        (sup[n-1] nao e usado)
 A sub e a superdiagonal NAO dependem de x: sao iguais em todas as iteracoes
 do Newton. Por isso sao preenchidas uma unica vez (jacobianaTriConst, antes
 do laco do Newton) e a cada iteracao so a diagonal e recalculada
 (jacobianaTri). Isso tira do laco um calculo que nao muda.*/

/*Preenche as diagonais constantes (sub e super). Chamada uma vez so.
 Como as posicoes nao usadas sao ignoradas pelo Gauss tridiagonal, o laco
 nao precisa de if para as bordas, o que permite a vetorizacao.*/
void jacobianaTriConst(double *restrict sub, double *restrict sup, int n) {

    for (int i = 0; i < n; ++i) {

        sub[i] = -1.0;
        sup[i] = -2.0;

    }

}

/*Recalcula a diagonal principal da Jacobiana no ponto x (a cada iteracao).*/
void jacobianaTri(const double *restrict x, double *restrict diag, int n) {

    for (int i = 0; i < n; ++i)
        diag[i] = -4.0 * x[i] + 3.0;

}
