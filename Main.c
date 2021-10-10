#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *criaArquivoEscrita(char *);
FILE *abreArquivoAtualizacao(char *);

int topoPed();
int byteOffsetApartirDoRNN(int);
void mostraRegistro(int);
int buscaRnnResgistro(char *);
void importacao(char *);
void executa_operacoes(char *);
void removeRegistroPelaChave(char *);

void importacao(char *argumentos) {
    FILE *arquivoCopia;
    FILE *candidatos;
    FILE *candidatosAux;

    arquivoCopia = criaArquivoEscrita("dados.dat");

    char letra, proxLetra;
    int contPipe = 0;
    int cont = 0;
    int cabeca = -1;

    candidatos = abreArquivoAtualizacao(argumentos);
    candidatosAux = abreArquivoAtualizacao(argumentos);

    letra = fgetc(candidatos);
    proxLetra = fgetc(candidatosAux);

    fwrite(&cabeca, sizeof(int), 1, arquivoCopia);

    while (!feof(candidatos)) {
        proxLetra = fgetc(candidatosAux);

        if (contPipe <= 3) {
            cont++;
            fputc(letra, arquivoCopia);
            letra = fgetc(candidatos);

        } else {
            fputc(letra, arquivoCopia);
            letra = fgetc(candidatos);

            while (cont <= 62) {
                fputc('\0', arquivoCopia);
                cont++;
            }

            cont = 0;
            contPipe = 0;
        }

        if (proxLetra == '|') {
            contPipe++;
        }
    }

    fclose(arquivoCopia);
    fclose(candidatos);
    fclose(candidatosAux);
}

void executa_operacoes(char *argumentos) {
    FILE *arquivoCopia;
    FILE *operacoes;
    char op;
    char busca[7], comparacao[7], excluiDado[7], elementoBuscado[64];
    int posicao = 0, codIgual = 0, encontrouChave = 0;
    int cabeca;

    operacoes = abreArquivoLeitura(argumentos);

    while (!feof(operacoes)) {
        op = fgetc(operacoes);

        if (op == 'b') {
            encontrouChave = 0;
            posicao = 0;

            arquivoCopia = abreArquivoLeitura("dados.dat");

            fgetc(operacoes);
            fgets(busca, 7, operacoes);
            printf("Busca pelo registro de chave %s\n", busca);
            while (!feof(arquivoCopia)) {
                fseek(arquivoCopia, (posicao * 64) + sizeof(int), SEEK_SET);
                fgets(comparacao, 7, arquivoCopia);
                if ((strcmp(busca, comparacao)) == 0) {
                    fgets(elementoBuscado, 64, arquivoCopia);
                    printf("%s%s(RRN = %d - byte-offset %ld)\n", busca, elementoBuscado, posicao, (posicao * 64) + sizeof(int));
                    // fseek(arquivoCopia, 0, SEEK_SET);
                    encontrouChave = 1;
                    posicao = 0;
                    break;
                } else {
                    posicao++;
                }
            }

            if (encontrouChave == 0) {
                printf("Chave invalida - Registro nao encontrado\n");
            }

            fclose(arquivoCopia);

        } else if (op == 'i') {
            arquivoCopia = abreArquivoLeitura("dados.dat");
            printf("INSERINDO\n");
            fclose(arquivoCopia);

        } else if (op == 'r') {
            cabeca = topoPed();

            encontrouChave = 0;
            posicao = 0;

            arquivoCopia = abreArquivoLeitura("dados.dat");

            fgetc(operacoes);
            fgets(excluiDado, 7, operacoes);
            printf("Remocao do registro de chave %s\n", excluiDado);
            while (!feof(arquivoCopia)) {
                fseek(arquivoCopia, (posicao * 64) + sizeof(int), SEEK_SET);
                fgets(comparacao, 7, arquivoCopia);
                if ((strcmp(excluiDado, comparacao)) == 0) {
                    fgets(elementoBuscado, 64, arquivoCopia);
                    printf("Posicao RRN = %d  (byte-offset %ld)\n", posicao, (posicao * 64) + sizeof(int));
                    fseek(arquivoCopia, (posicao * 64) + sizeof(int), SEEK_SET);
                    //fprintf(arquivoCopia, "*%d", cabeca);
                    fwrite(&cabeca, sizeof(int), 1, arquivoCopia);
                    fseek(arquivoCopia, 0, SEEK_SET);
                    // fprintf(arquivoCopia, "%d", posicao);
                    fwrite(&posicao, sizeof(int), 1, arquivoCopia);
                    encontrouChave = 1;
                    posicao = 0;
                    break;
                } else {
                    posicao++;
                }
            }

            if (encontrouChave == 0) {
                printf("Chave invalida - Registro nao encontrado\n");
            }

            fclose(arquivoCopia);
        }
    }
}

int main(int numeroArgumentos, char *argumentos[]) {
    if (numeroArgumentos == 3 && strcmp(argumentos[1], "-i") == 0) {
        printf("Modo de importacao ativado ... nome do arquivoCopia = %s\n", argumentos[2]);

        importacao(argumentos[2]);

    } else if (numeroArgumentos == 3 && strcmp(argumentos[1], "-e") == 0) {
        printf("Modo de execucao de operacoes ativado ... nome do arquivoCopia = %s\n", argumentos[2]);
        executa_operacoes(argumentos[2]);

    } else if (numeroArgumentos == 2 && strcmp(argumentos[1], "-p") == 0) {
        printf("Modo de impressao da PED ativado ...\n");
        //imprime_ped();

    } else {
        fprintf(stderr, "Argumentos incorretos!\n");
        fprintf(stderr, "Modo de uso:\n");
        fprintf(stderr, "$ %s (-i|-e) nome_arquivoCopia\n", argumentos[0]);
        fprintf(stderr, "$ %s -p\n", argumentos[0]);
        exit(EXIT_FAILURE);
    }

    return 0;
}