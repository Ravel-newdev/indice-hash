#include "hash.h"

/* Nome do arquivo de dados (dataset) */
#define ARQUIVO_TEXTO "bd-trab2-dataset.csv"
/* Nome do arquivo do diretório persistido */
#define ARQUIVO_DIR   "diretorio.txt"

/* ─────────────────────────────────────────────────────────────
 * Função hash: retorna os PG bits menos significativos de key
 * Ex: key=6 (110b), PG=2 → 6 & (4-1) = 6 & 3 = 2 (10b)
 * ───────────────────────────────────────────────────────────── */
int calcularHash(int linhaNum, int profGlobal) {
    int mascara = (1 << profGlobal) - 1; /* 2^PG - 1 */
    return linhaNum & mascara;
}

/* ─────────────────────────────────────────────────────────────
 * Cria um diretório novo com profundidade global inicial.
 * Cria também os buckets iniciais (2^PG buckets).
 * ───────────────────────────────────────────────────────────── */
Diretorio* criarDiretorio(int profGlobalInicial) {
    Diretorio *dir = (Diretorio*) malloc(sizeof(Diretorio));
    dir->profGlobal    = profGlobalInicial;
    dir->numEntradas   = 1 << profGlobalInicial; /* 2^PG */
    dir->bucketIds     = (int*) malloc(dir->numEntradas * sizeof(int));
    dir->proximoBucketId = 0;

    /* Cria um bucket para cada entrada do diretório */
    for (int i = 0; i < dir->numEntradas; i++) {
        dir->bucketIds[i] = dir->proximoBucketId;
        Bucket *b = criarBucket(profGlobalInicial);
        salvarBucket(dir->proximoBucketId, b);
        free(b);
        dir->proximoBucketId++;
    }
    return dir;
}

/* ─────────────────────────────────────────────────────────────
 * Libera memória do diretório
 * ───────────────────────────────────────────────────────────── */
void liberarDiretorio(Diretorio *dir) {
    if (dir) {
        free(dir->bucketIds);
        free(dir);
    }
}

/* ─────────────────────────────────────────────────────────────
 * Cria um bucket vazio com profundidade local dada
 * ───────────────────────────────────────────────────────────── */
Bucket* criarBucket(int profLocal) {
    Bucket *b = (Bucket*) malloc(sizeof(Bucket));
    b->profLocal   = profLocal;
    b->numEntradas = 0;
    memset(b->entradas, 0, sizeof(b->entradas));
    return b;
}

/* ─────────────────────────────────────────────────────────────
 * Lê um bucket do arquivo correspondente.
 * Formato do arquivo bucket_<id>.txt:
 *   linha 1: PL
 *   linha 2: numEntradas
 *   linhas seguintes: um LinhaNum por linha
 * ───────────────────────────────────────────────────────────── */
Bucket* lerBucket(int bucketId) {
    char nomeArq[64];
    sprintf(nomeArq, "%s%d.txt", BUCKET_PREFIX, bucketId);

    FILE *f = fopen(nomeArq, "r");
    if (!f) {
        /* Bucket ainda não existe — retorna vazio com PL=1 */
        return criarBucket(1);
    }

    Bucket *b = (Bucket*) malloc(sizeof(Bucket));
    fscanf(f, "%d", &b->profLocal);
    fscanf(f, "%d", &b->numEntradas);
    for (int i = 0; i < b->numEntradas; i++) {
        fscanf(f, "%d", &b->entradas[i].linhaNum);
    }
    fclose(f);
    return b;
}

/* ─────────────────────────────────────────────────────────────
 * Salva um bucket no arquivo correspondente (1 página na RAM)
 * ───────────────────────────────────────────────────────────── */
void salvarBucket(int bucketId, Bucket *b) {
    char nomeArq[64];
    sprintf(nomeArq, "%s%d.txt", BUCKET_PREFIX, bucketId);

    FILE *f = fopen(nomeArq, "w");
    if (!f) {
        fprintf(stderr, "Erro ao salvar bucket %d\n", bucketId);
        return;
    }
    fprintf(f, "%d\n", b->profLocal);
    fprintf(f, "%d\n", b->numEntradas);
    for (int i = 0; i < b->numEntradas; i++) {
        fprintf(f, "%d\n", b->entradas[i].linhaNum);
    }
    fclose(f);
}

/* ─────────────────────────────────────────────────────────────
 * Salva diretório em arquivo texto
 * Formato:
 *   PG
 *   numEntradas
 *   proximoBucketId
 *   bucketId[0] bucketId[1] ... bucketId[n-1]
 * ───────────────────────────────────────────────────────────── */
void salvarDiretorio(Diretorio *dir) {
    FILE *f = fopen(ARQUIVO_DIR, "w");
    if (!f) { fprintf(stderr, "Erro ao salvar diretório\n"); return; }

    fprintf(f, "%d\n", dir->profGlobal);
    fprintf(f, "%d\n", dir->numEntradas);
    fprintf(f, "%d\n", dir->proximoBucketId);
    for (int i = 0; i < dir->numEntradas; i++) {
        fprintf(f, "%d\n", dir->bucketIds[i]);
    }
    fclose(f);
}

/* ─────────────────────────────────────────────────────────────
 * Carrega diretório do arquivo (se existir)
 * ───────────────────────────────────────────────────────────── */
Diretorio* carregarDiretorio(void) {
    FILE *f = fopen(ARQUIVO_DIR, "r");
    if (!f) return NULL;

    Diretorio *dir = (Diretorio*) malloc(sizeof(Diretorio));
    fscanf(f, "%d", &dir->profGlobal);
    fscanf(f, "%d", &dir->numEntradas);
    fscanf(f, "%d", &dir->proximoBucketId);
    dir->bucketIds = (int*) malloc(dir->numEntradas * sizeof(int));
    for (int i = 0; i < dir->numEntradas; i++) {
        fscanf(f, "%d", &dir->bucketIds[i]);
    }
    fclose(f);
    return dir;
}

/* ─────────────────────────────────────────────────────────────
 * Duplica o diretório (dobra o número de entradas)
 * Cada entrada antiga vira duas entradas apontando para o mesmo bucket
 * ───────────────────────────────────────────────────────────── */
void duplicarDiretorio(Diretorio *dir) {
    int novoNum = dir->numEntradas * 2;
    int *novoIds = (int*) malloc(novoNum * sizeof(int));

    /* Cada par (i, i + numEntradas/2) aponta para o mesmo bucket antigo */
    for (int i = 0; i < dir->numEntradas; i++) {
        novoIds[i]                    = dir->bucketIds[i];
        novoIds[i + dir->numEntradas] = dir->bucketIds[i];
    }

    free(dir->bucketIds);
    dir->bucketIds   = novoIds;
    dir->numEntradas = novoNum;
    dir->profGlobal++;
}

/* ─────────────────────────────────────────────────────────────
 * Split do bucket apontado pelo índice do diretório.
 * - Cria novo bucket com id = proximoBucketId
 * - Re-hash de todos os elementos do bucket cheio
 * - Atualiza ponteiros no diretório
 * ───────────────────────────────────────────────────────────── */
void splitBucket(Diretorio *dir, int indiceDiretorio) {
    int idAntigo = dir->bucketIds[indiceDiretorio];
    Bucket *bAntigo = lerBucket(idAntigo); /* 1 página na RAM */

    /* Nova profundidade local */
    int novaProf = bAntigo->profLocal + 1;

    /* Cria bucket novo */
    int idNovo = dir->proximoBucketId++;
    Bucket *bNovo = criarBucket(novaProf);
    bAntigo->profLocal = novaProf;

    /* Guarda entradas antigas para redistribuir */
    Entrada antigas[MAX_BUCKET_SIZE];
    int qtdAntiga = bAntigo->numEntradas;
    memcpy(antigas, bAntigo->entradas, qtdAntiga * sizeof(Entrada));

    /* Esvazia bucket antigo */
    bAntigo->numEntradas = 0;

    /* Atualiza ponteiros do diretório:
     * As entradas cujos bits PL apontam para o bucket novo recebem idNovo.
     * A máscara usa novaProf bits. */
    int mascara = (1 << novaProf) - 1;

    for (int i = 0; i < dir->numEntradas; i++) {
        if (dir->bucketIds[i] == idAntigo) {
            /* Verifica qual bucket deve receber esta entrada do diretório */
            int sufixo = i & mascara;
            /* Bit diferenciador: bit na posição (novaProf-1) */
            if (sufixo & (1 << (novaProf - 1))) {
                dir->bucketIds[i] = idNovo;
            }
            /* else mantém idAntigo */
        }
    }

    /* Re-distribui as entradas antigas nos dois buckets */
    for (int i = 0; i < qtdAntiga; i++) {
        int h = calcularHash(antigas[i].linhaNum, novaProf) & mascara;
        /* Descobre qual bucket recebe esta entrada */
        if (h & (1 << (novaProf - 1))) {
            bNovo->entradas[bNovo->numEntradas++] = antigas[i];
        } else {
            bAntigo->entradas[bAntigo->numEntradas++] = antigas[i];
        }
    }

    /* Persiste os dois buckets (1 por vez) */
    salvarBucket(idAntigo, bAntigo);
    free(bAntigo);
    salvarBucket(idNovo, bNovo);
    free(bNovo);
}

/* ─────────────────────────────────────────────────────────────
 * Busca o registro no ArquivoTexto usando LinhaNum como RID.
 * Lê apenas 1 linha por vez (1 página na memória).
 * ───────────────────────────────────────────────────────────── */
void buscarNoArquivo(int linhaNum) {
    FILE *f = fopen(ARQUIVO_TEXTO, "r");
    if (!f) { fprintf(stderr, "Arquivo de dados não encontrado.\n"); return; }

    char linha[MAX_LINHA_TEXT];
    /* pula cabeçalho */
    fgets(linha, sizeof(linha), f);

    while (fgets(linha, sizeof(linha), f)) {
        /* Remove \r\n */
        linha[strcspn(linha, "\r\n")] = 0;

        /* Lê key */
        char *virgula = strchr(linha, ',');
        if (!virgula) continue;
        *virgula = '\0';
        int key = atoi(linha);

        if (key == linhaNum) {
            /* Encontrou — restaura e imprime */
            printf("  Registro encontrado: %d | %s\n", key, virgula + 1);
            fclose(f);
            return;
        }
        /* 1 linha por vez: não mantemos nada em memória além desta */
    }
    fclose(f);
}

/* ─────────────────────────────────────────────────────────────
 * INSERÇÃO
 * ───────────────────────────────────────────────────────────── */
int inserir(Diretorio *dir, int linhaNum, FILE *outFile) {
    /* Loop: tenta inserir até conseguir (pode precisar de splits) */
    while (1) {
        int h = calcularHash(linhaNum, dir->profGlobal);
        int bucketId = dir->bucketIds[h];

        /* Carrega 1 bucket na RAM */
        Bucket *b = lerBucket(bucketId);

        /* Verifica se já existe (duplicata) */
        for (int i = 0; i < b->numEntradas; i++) {
            if (b->entradas[i].linhaNum == linhaNum) {
                /* Já existe — apenas reporta */
                fprintf(outFile, "INC:%d/%d,%d\n",
                        linhaNum, dir->profGlobal, b->profLocal);
                free(b);
                return 0;
            }
        }

        if (b->numEntradas < MAX_BUCKET_SIZE) {
            /* Espaço disponível — insere */
            b->entradas[b->numEntradas].linhaNum = linhaNum;
            b->numEntradas++;
            salvarBucket(bucketId, b);

            fprintf(outFile, "INC:%d/%d,%d\n",
                    linhaNum, dir->profGlobal, b->profLocal);
            free(b);
            return 0;
        }

        /* Bucket cheio — precisa de split */
        int profLocalAtual = b->profLocal;
        free(b);

        if (profLocalAtual == dir->profGlobal) {
            /* PL == PG: duplica diretório antes do split */
            duplicarDiretorio(dir);
            /* Recalcula h com novo PG */
            h = calcularHash(linhaNum, dir->profGlobal);
            /* Registra duplicação no outFile ANTES do INC */
            /* (será escrito pelo chamador em main.c via flag) */
            /* Aqui escrevemos direto conforme enunciado */
            /* Busca PL do bucket que vai ser splitado */
            Bucket *bCheck = lerBucket(dir->bucketIds[h]);
            fprintf(outFile, "DUP DIR:/%d,%d\n",
                    dir->profGlobal, bCheck->profLocal + 1);
            free(bCheck);
        }

        /* Faz split do bucket referenciado pelo índice h */
        h = calcularHash(linhaNum, dir->profGlobal);
        splitBucket(dir, h);
        /* Tenta inserir novamente no próximo loop */
    }
}

/* ─────────────────────────────────────────────────────────────
 * REMOÇÃO
 * ───────────────────────────────────────────────────────────── */
int remover(Diretorio *dir, int linhaNum, FILE *outFile) {
    int h = calcularHash(linhaNum, dir->profGlobal);
    int bucketId = dir->bucketIds[h];

    /* Carrega 1 bucket na RAM */
    Bucket *b = lerBucket(bucketId);

    int removidos = 0;
    /* Percorre entradas e remove as que batem */
    for (int i = 0; i < b->numEntradas; i++) {
        if (b->entradas[i].linhaNum == linhaNum) {
            /* Remove deslocando os elementos seguintes */
            for (int j = i; j < b->numEntradas - 1; j++) {
                b->entradas[j] = b->entradas[j + 1];
            }
            b->numEntradas--;
            removidos++;
            i--; /* re-verifica posição atual */
        }
    }

    salvarBucket(bucketId, b);
    fprintf(outFile, "REM:%d/%d,%d,%d\n",
            linhaNum, removidos, dir->profGlobal, b->profLocal);
    free(b);
    return 0;
}

/* ─────────────────────────────────────────────────────────────
 * BUSCA POR IGUALDADE
 * ───────────────────────────────────────────────────────────── */
int buscarIgualdade(Diretorio *dir, int linhaNum, FILE *outFile) {
    int h = calcularHash(linhaNum, dir->profGlobal);
    int bucketId = dir->bucketIds[h];

    /* Carrega 1 bucket na RAM */
    Bucket *b = lerBucket(bucketId);

    int encontrados = 0;
    for (int i = 0; i < b->numEntradas; i++) {
        if (b->entradas[i].linhaNum == linhaNum) {
            encontrados++;
            buscarNoArquivo(linhaNum);
        }
    }

    fprintf(outFile, "BUS:%d/%d\n", linhaNum, encontrados);
    free(b);
    return 0;
}
