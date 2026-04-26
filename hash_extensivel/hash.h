#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUCKET_SIZE   5      // máximo de entradas por bucket   
#define MAX_LINHA_TEXT  512      // tamanho máximo do texto de linha 
#define BUCKET_PREFIX   "bucket_" // prefixo dos arquivos de bucket 


// Entrada de dados dentro de um bucket 
typedef struct {
    int linhaNum;   // identificador da linha (também é o RID)
} Entrada;

// Bucket: profundidade local + vetor de entradas 
typedef struct {
    int profLocal;              // profundidade local (PL) 
    int numEntradas;            // quantidade de entradas no momento
    Entrada entradas[MAX_BUCKET_SIZE];
} Bucket;

// Diretório do hash extensível
typedef struct {
    int profGlobal;             // profundidade global (PG) 
    int numEntradas;            // 2^PG entradas no diretório 
    int *bucketIds;             // vetor de IDs de bucket (índice → id do arquivo) 
    int proximoBucketId;        // próximo ID disponível para novo bucket 
} Diretorio;


// Inicialização
Diretorio* criarDiretorio(int profGlobalInicial);
void liberarDiretorio(Diretorio *dir);

// I/O de buckets (1 página por vez em memória)
Bucket* lerBucket(int bucketId);
void salvarBucket(int bucketId, Bucket *b);
Bucket* criarBucket(int profLocal);

// Função hash: retorna os PG bits menos significativos de linhaNum 
int calcularHash(int linhaNum, int profGlobal);

// Operações principais 
// Retorna: 0=ok, -1=erro 
int inserir(Diretorio *dir, int linhaNum, FILE *outFile);
int remover(Diretorio *dir, int linhaNum, FILE *outFile);
int buscarIgualdade(Diretorio *dir, int linhaNum, FILE *outFile);

// Busca no ArquivoTexto e imprime o registro
void buscarNoArquivo(int linhaNum);

// I/O do diretório em arquivo 
void salvarDiretorio(Diretorio *dir);
Diretorio* carregarDiretorio(void);

// Duplicação do diretório quando PL == PG
void duplicarDiretorio(Diretorio *dir);

// Split de bucket cheio 
void splitBucket(Diretorio *dir, int indiceBucket);

#endif
