# Índice Hash Extensível — Trabalho 2 SGBD

## Descrição

Implementação de um **índice hash extensível** em C para indexar registros do livro Dom Casmurro armazenados em disco. O índice suporta busca por igualdade, inserção e remoção de entradas.

## Estrutura do projeto

```
.
├── main.c               # Ponto de entrada: lê in.txt, executa operações, gera out.txt
├── hash.h               # Definições de estruturas e protótipos
├── hash.c               # Implementação do hash extensível
├── Makefile             # Compilação
├── bd-trab2-dataset.csv # Arquivo de dados (Dom Casmurro)
├── in.txt               # Arquivo de entrada com operações
└── out.txt              # Arquivo de saída gerado automaticamente
```

## Arquivos gerados em execução

| Arquivo | Descrição |
|---|---|
| `diretorio.txt` | Persistência do diretório hash (PG + vetor de IDs de bucket) |
| `bucket_N.txt` | Um arquivo por bucket (profundidade local + entradas) |
| `out.txt` | Resultado das operações |

## Estrutura do índice

- **Diretório**: vetor de 2^PG ponteiros (IDs) para buckets + profundidade global (PG)
- **Bucket**: até 5 entradas (LinhaNum) + profundidade local (PL)
- **Função hash**: `LinhaNum & (2^PG - 1)` — bits menos significativos
- **LinhaNum** atua como chave de busca e RID (localizador direto no arquivo de dados)

## Regra de memória

Apenas **uma página (bucket) por vez** é carregada na RAM. O diretório é exceção e fica em memória durante toda a execução.

## Como compilar

```bash
make
```

## Como executar

1. Coloque o arquivo `bd-trab2-dataset.csv` na mesma pasta do executável
2. Crie o arquivo `in.txt` com as operações (veja formato abaixo)
3. Execute:

```bash
./hash_extensivel
```

O resultado será gerado em `out.txt`.

## Formato do in.txt

```
PG/<profundidade global inicial>
INC:x        # inserir x no índice
REM:x        # remover x do índice
BUS=:x       # buscar x por igualdade
```

**Exemplo:**
```
PG/2
INC:5
INC:9
BUS=:5
REM:9
BUS=:9
```

## Formato do out.txt

```
PG/<pg>
INC:x/<pg>,<pl>
DUP DIR:/<pg>,<pl>          # aparece quando houve duplicação do diretório
REM:x/<qtd removida>,<pg>,<pl>
BUS:x/<qtd encontrada>
P:/<pg final>
```

## Limpeza

```bash
make clean
```

Remove objetos, executável, buckets, diretório e out.txt.
