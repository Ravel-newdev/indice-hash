#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

/*
  main.c — ponto de entrada do índice hash extensível
 
  Lê operações de in.txt, executa no índice e escreve out.txt.
 
  Formato de in.txt:
    PG/<profundidade global inicial>
    INC:x | REM:x | BUS=:x
 
  Formato de out.txt:
    PG/<pg>
    INC:x/<pg>,<pl>
    DUP DIR:/<pg>,<pl>   (quando houve duplicação)
    REM:x/<qtd>,<pg>,<pl>
    BUS:x/<qtd>
    P:/<pg final>
 */
int main(void) {
    FILE *inFile  = fopen("in.txt",  "r");
    FILE *outFile = fopen("out.txt", "w");

    if (!inFile)  { fprintf(stderr, "Erro: in.txt não encontrado.\n");  return 1; }
    if (!outFile) { fprintf(stderr, "Erro: não foi possível criar out.txt.\n"); return 1; }

    char linha[256];

    // Lê primeira linha: PG/<profundidade>
    if (!fgets(linha, sizeof(linha), inFile)) {
        fprintf(stderr, "Arquivo de entrada vazio.\n");
        fclose(inFile); fclose(outFile);
        return 1;
    }
    linha[strcspn(linha, "\r\n")] = 0;

    int profGlobalInicial = 0;
    if (sscanf(linha, "PG/%d", &profGlobalInicial) != 1) {
        fprintf(stderr, "Formato inválido na 1ª linha: %s\n", linha);
        fclose(inFile); fclose(outFile);
        return 1;
    }

    // Escreve primeira linha no out.txt (igual à entrada)
    fprintf(outFile, "%s\n", linha);

    // Cria/carrega diretório
    Diretorio *dir = criarDiretorio(profGlobalInicial);

    // Processa operações linha a linha
    while (fgets(linha, sizeof(linha), inFile)) {
        linha[strcspn(linha, "\r\n")] = 0;
        if (strlen(linha) == 0) continue; // ignora linhas vazias

        int x = 0;

        if (sscanf(linha, "INC:%d", &x) == 1) {
            // Inserção
            inserir(dir, x, outFile);

        } else if (sscanf(linha, "REM:%d", &x) == 1) {
            // Remoção
            remover(dir, x, outFile);

        } else if (sscanf(linha, "BUS=:%d", &x) == 1) {
            // Busca por igualdade
            buscarIgualdade(dir, x, outFile);

        } else {
            fprintf(stderr, "Operação desconhecida: %s\n", linha);
        }
    }

    // Última linha: profundidade global final
    fprintf(outFile, "P:/%d\n", dir->profGlobal);

    // Persiste diretório e libera memória
    salvarDiretorio(dir);
    liberarDiretorio(dir);

    fclose(inFile);
    fclose(outFile);

    printf("Processamento concluído. Veja out.txt.\n");
    return 0;
}
