# Gera os 12 graficos do relatorio (3 trechos x 4 testes) a partir de
# graficos/dados.csv, em graficos/figuras/*.pdf
#
# Uso (na raiz do projeto, depois do extrai_dados.py):
#   python graficos/gera_graficos.py

import csv
import matplotlib
matplotlib.use('Agg')            # so salva os arquivos, sem abrir janela
import matplotlib.pyplot as plt

NS = [32, 64, 128, 256, 512, 1000, 2000, 4000, 8000, 9000, 10000, 20000]

# Uma cor para cada versao, igual em todos os graficos
VERSOES = {'nao-otimizado': ('Não otimizado', 'tab:red'),
           'otimizado':     ('Otimizado',     'tab:blue')}

TRECHOS = {'newton':    'Método de Newton (total)',
           'jacobiana': 'Cálculo das Jacobianas',
           'sl':        'Resolução dos sistemas lineares'}


def le_dados():
    """Le o dados.csv e devolve uma lista de linhas (dicionarios)."""
    with open('graficos/dados.csv') as f:
        return list(csv.DictReader(f))


def serie(dados, versao, regiao, coluna):
    """Devolve as listas (N, valor) de uma versao/regiao, pulando valores vazios."""
    xs, ys = [], []
    for linha in dados:
        if linha['versao'] == versao and linha['regiao'] == regiao and linha[coluna] != '':
            xs.append(int(linha['N']))
            ys.append(float(linha[coluna]))
    return xs, ys


def finaliza(titulo, rotulo_y, log_y, arquivo, nota=None):
    """Eixos, legenda e grade comuns a todos os graficos; salva o PDF."""
    plt.xscale('log')                       # eixo N em escala log (enunciado)
    plt.xticks(NS, NS, rotation=90, fontsize=8)   # 8000, 9000 e 10000 ficam proximos no log
    plt.minorticks_off()
    if log_y:
        plt.yscale('log')
    plt.xlabel('N (tamanho do sistema)')
    plt.ylabel(rotulo_y)
    plt.title(titulo)
    plt.grid(True, alpha=0.3)
    plt.legend()
    if nota:
        plt.figtext(0.01, 0.01, nota, fontsize=10)
        plt.tight_layout(rect=[0, 0.06, 1, 1])   # deixa espaco embaixo para a nota
    else:
        plt.tight_layout()
    plt.savefig('graficos/figuras/' + arquivo)
    # versao PNG para inserir no relatorio (Word)
    plt.savefig('graficos/figuras/' + arquivo.replace('.pdf', '.png'), dpi=200)
    plt.close()


def grafico(dados, regiao, coluna, teste, rotulo_y, log_y, arquivo, nota=None):
    """Grafico de uma metrica (uma linha por versao)."""
    plt.figure(figsize=(8, 4.5))
    for versao, (nome, cor) in VERSOES.items():
        xs, ys = serie(dados, versao, regiao, coluna)
        plt.plot(xs, ys, marker='o', color=cor, label=nome)
    finaliza(teste + ' – ' + TRECHOS[regiao], rotulo_y, log_y, arquivo, nota)


def grafico_flops(dados, regiao):
    """MFLOP/s: FLOPS_DP (linha cheia) e FLOPS_AVX (tracejada) de cada versao.
    Eixo Y em log, porque os valores vao de ~1 a ~8000 MFLOP/s. Zero nao
    aparece em escala log: uma serie de FLOPS_AVX toda zerada (codigo sem
    vetorizacao AVX) nao e desenhada e fica indicada na nota do grafico."""
    plt.figure(figsize=(8, 4.5))
    zerados = []
    for versao, (nome, cor) in VERSOES.items():
        xs, ys = serie(dados, versao, regiao, 'dp_mflops')
        plt.plot(xs, ys, marker='o', color=cor, label='FLOPS_DP (' + nome + ')')
        xs, ys = serie(dados, versao, regiao, 'avx_mflops')
        if max(ys) == 0.0:
            zerados.append(nome.lower().replace('otimizado', 'otimizada'))
        else:
            plt.plot(xs, ys, marker='s', color=cor, linestyle='--',
                     label='FLOPS_AVX (' + nome + ')')
    nota = None
    if zerados:
        nota = 'FLOPS_AVX = 0 em todos os N na versão ' + ' e '.join(zerados) +                ' (não aparece em escala log).'
    finaliza('Operações aritméticas – ' + TRECHOS[regiao], 'MFLOP/s', True,
             'flops_' + regiao + '.pdf', nota)


def main():
    dados = le_dados()
    for regiao in TRECHOS:
        grafico(dados, regiao, 'tempo_ms', 'Tempo de execução',
                'Tempo médio (ms)', True, 'tempo_' + regiao + '.pdf')

        nota = None
        if regiao == 'newton':
            nota = 'Não otimizado: N > 1000 omitido (estouro do contador do grupo MEM).'
        grafico(dados, regiao, 'mem_mbs', 'Banda de memória',
                'Memory bandwidth [MBytes/s]', True, 'mem_' + regiao + '.pdf', nota)

        grafico(dados, regiao, 'l2_miss', 'Cache miss L2',
                'L2 miss ratio', False, 'l2_' + regiao + '.pdf')

        grafico_flops(dados, regiao)
    print('12 graficos gerados em graficos/figuras/')


main()
