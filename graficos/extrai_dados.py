# Extrai os dados das coletas (tempos.csv e saidas do LIKWID) e junta tudo
# em uma tabela so: graficos/dados.csv
#
# Uso (na raiz do projeto, depois de extrair os .tar.gz de resultados_coleta/):
#   python graficos/extrai_dados.py

import csv

# Pastas extraidas de resultados_coleta/*.tar.gz
PASTAS = {'nao-otimizado': 'resultados_nao-otimizado',
          'otimizado':     'resultados_otimizado'}

# Valores de N do enunciado (a versao nao otimizada so foi medida ate 4000)
NS = [32, 64, 128, 256, 512, 1000, 2000, 4000, 8000, 9000, 10000, 20000]

REGIOES = ['newton', 'jacobiana', 'sl']

# Coluna do tempos.csv correspondente a cada regiao
COLUNA_TEMPO = {'newton': 'total_ms', 'jacobiana': 'jacobiana_ms', 'sl': 'sl_ms'}

# Grupo do LIKWID -> metrica pedida no enunciado (nome exato na saida do LIKWID)
METRICAS = {'MEM':       'Memory bandwidth [MBytes/s]',
            'L2CACHE':   'L2 miss ratio',
            'FLOPS_DP':  'DP [MFLOP/s]',
            'FLOPS_AVX': 'Packed DP [MFLOP/s]'}


def le_tempos(pasta):
    """Le o tempos.csv e devolve {(N, regiao): media das repeticoes em ms}."""
    soma = {}
    qtd = {}
    with open(pasta + '/tempos.csv') as f:
        for linha in csv.DictReader(f):
            n = int(linha['N'])
            for r in REGIOES:
                chave = (n, r)
                soma[chave] = soma.get(chave, 0.0) + float(linha[COLUNA_TEMPO[r]])
                qtd[chave] = qtd.get(chave, 0) + 1
    return {chave: soma[chave] / qtd[chave] for chave in soma}


def le_metrica(arquivo, metrica):
    """Le uma saida do likwid-perfctr e devolve {regiao: valor da metrica}.
    Cada regiao comeca com uma linha 'Region <nome>, Group ...' e as metricas
    ficam em tabelas no formato '| <nome da metrica> | <valor> |'."""
    valores = {}
    regiao = None
    with open(arquivo) as f:
        for linha in f:
            if linha.startswith('Region '):
                regiao = linha.split()[1].rstrip(',')
            elif '|' in linha and regiao is not None:
                colunas = [c.strip() for c in linha.split('|')]
                if colunas[1] == metrica:
                    valores[regiao] = float(colunas[2])
    return valores


def main():
    with open('graficos/dados.csv', 'w', newline='') as f:
        saida = csv.writer(f)
        saida.writerow(['versao', 'regiao', 'N', 'tempo_ms', 'mem_mbs',
                        'l2_miss', 'dp_mflops', 'avx_mflops'])

        for versao, pasta in PASTAS.items():
            tempos = le_tempos(pasta)
            for n in NS:
                if (n, 'newton') not in tempos:   # N nao medido nesta versao
                    continue

                # le os 4 grupos deste N
                m = {}
                for grupo, metrica in METRICAS.items():
                    arquivo = '%s/raw/%s_%s_%d.txt' % (pasta, versao, grupo, n)
                    m[grupo] = le_metrica(arquivo, metrica)

                for r in REGIOES:
                    mem = m['MEM'][r]
                    # O contador do grupo MEM tem 32 bits e da a volta a cada
                    # ~274,9 GB. Na versao nao otimizada, a regiao newton (uma
                    # unica chamada) passa disso em N = 2000 e 4000, entao o
                    # valor medido esta errado: fica de fora do grafico.
                    if versao == 'nao-otimizado' and r == 'newton' and n > 1000:
                        mem = ''
                    saida.writerow([versao, r, n, tempos[(n, r)], mem,
                                    m['L2CACHE'][r], m['FLOPS_DP'][r],
                                    m['FLOPS_AVX'][r]])

    print('graficos/dados.csv gerado')


main()
