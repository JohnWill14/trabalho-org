#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *criaArquivoEscrita(char *);
FILE *abreArquivoAtualizacao(char *);

int topoPed();
int proxPed(int);
int byteOffsetApartirDoRNN(int);
void mostraRegistro(int);
int buscaRnnResgistro(char *);
void importacao(char *);
void leiaRegistro(char *, FILE *);
void inseriRegistro(char *);
void executa_operacoes(char *);
void removeRegistroPelaChave(char *);
void imprime_ped();

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

void executa_operacoes(char *nomeArquivoOperacao) {
    FILE *operacoes;

    char op;
    char busca[7];

    operacoes = abreArquivoAtualizacao(nomeArquivoOperacao);

    while (!feof(operacoes)) {
        op = fgetc(operacoes);

        if (op == 'b') {
            fgetc(operacoes);
            fgets(busca, 7, operacoes);

            int posicaoRegistro = buscaRnnResgistro(busca);

            printf("Busca pelo registro de chave \"%s\"\n", busca);
            if (posicaoRegistro != -1) {
                mostraRegistro(posicaoRegistro);
            } else {
                printf("    CHAVE INVALIDA - Registro nao encontrado\n");
            }

        } else if (op == 'r') {
            fgetc(operacoes);
            fgets(busca, 7, operacoes);

            printf("REMOCAO pelo registro de chave \"%s\"\n", busca);
            removeRegistroPelaChave(busca);
        } else if (op == 'i') {
            fgetc(operacoes);
            char registro[64];
            char chave[8];

            leiaRegistro(registro, operacoes);

            for (int i = 0; i < 6; i++) {
                chave[i] = registro[i];
            }
            chave[7] = '\0';

            printf("Insercao do registro de chave \"%s\"\n", chave);

            inseriRegistro(registro);
        }
        if (op == 'i' || op == 'b' || op == 'r') {
            puts("---\n");
        }
    }
}

void leiaRegistro(char registro[64], FILE *arquivo) {
    int contPipe = 0;
    int tamanhoRegistro = 0;

    while (tamanhoRegistro < 64) {
        if (contPipe < 4) {
            registro[tamanhoRegistro] = fgetc(arquivo);

            if (registro[tamanhoRegistro] == '|') {
                contPipe += 1;
            }
        } else {
            registro[tamanhoRegistro] = '\0';
        }

        tamanhoRegistro += 1;
    }
}

void inseriRegistro(char *registro) {
    int cabeca = topoPed();
    int proxEspaco = proxPed(cabeca);

    FILE *dados = abreArquivoAtualizacao("dados.dat");

    if (cabeca == -1) {
        puts("  Inseri no fim do arquivo");
        fseek(dados, 0, SEEK_END);
        fwrite(registro, sizeof(char), 64, dados);
    } else {
        printf("    Local: RRN = %d (byte-offset %d) [reutilizado]\n", cabeca, byteOffsetApartirDoRNN(cabeca));
        fwrite(&proxEspaco, sizeof(int), 1, dados);
        fseek(dados, byteOffsetApartirDoRNN(cabeca), SEEK_SET);
        fwrite(registro, sizeof(char), 64, dados);
    }

    fclose(dados);
}

void removeRegistroPelaChave(char *chave) {
    FILE *dados = abreArquivoAtualizacao("dados.dat");
    int posicaoRegistro = buscaRnnResgistro(chave);
    int cabeca = topoPed();

    if (posicaoRegistro == -1) {
        printf("    CHAVE INVALIDA - Registro nao encontrado\n");
        return;
    }

    printf("    Registro removido!\n");
    printf("    Posicao: RRN = %d (byte-offset %d)\n", posicaoRegistro, byteOffsetApartirDoRNN(posicaoRegistro));

    fwrite(&posicaoRegistro, sizeof(int), 1, dados);
    fseek(dados, byteOffsetApartirDoRNN(posicaoRegistro), SEEK_SET);
    fputc('*', dados);
    fwrite(&cabeca, sizeof(int), 1, dados);

    fclose(dados);
}

int buscaRnnResgistro(char *chave) {
    FILE *dados = abreArquivoAtualizacao("dados.dat");
    int posicao = 0;
    char chaveDoRegistro[7];
    char registro;
    bool encontrouChave = false;

    while (!feof(dados)) {
        fseek(dados, byteOffsetApartirDoRNN(posicao), SEEK_SET);
        fgets(chaveDoRegistro, 7, dados);

        if ((strcmp(chave, chaveDoRegistro)) == 0) {
            encontrouChave = true;
            break;
        } else {
            posicao++;
        }
    }

    if (!encontrouChave) {
        posicao = -1;
    }

    fclose(dados);

    return posicao;
}

void mostraRegistro(int posicao) {
    FILE *dados = abreArquivoAtualizacao("dados.dat");
    char buffer[64];
    int byteoffset;

    byteoffset = byteOffsetApartirDoRNN(posicao);

    fseek(dados, byteoffset, SEEK_SET);
    fread(buffer, sizeof(char), 64, dados);

    printf("    %s - (RNN: %d, byte-offset: %d )\n", buffer, posicao, byteoffset);

    fclose(dados);
}

int topoPed() {
    FILE *arquivoCopia;
    int topoPed;

    arquivoCopia = abreArquivoAtualizacao("dados.dat");

    fread(&topoPed, sizeof(int), 1, arquivoCopia);

    fclose(arquivoCopia);

    return topoPed;
}

int byteOffsetApartirDoRNN(int rnn) {
    return (rnn * 64) + sizeof(int);
}

FILE *criaArquivoEscrita(char *nomeArquivo) {
    /*
        Ambos r+e w+podem ler e gravar em um arquivo. No entanto, r+ não exclui o conteúdo do
        arquivo e não cria um novo arquivo se tal arquivo não existir, enquanto w+ exclui o 
        conteúdo do arquivo e o cria se ele não existir.

        URL: https://stackoverflow.com/questions/21113919/difference-between-r-and-w-in-fopen
    */
    FILE *arquivo = fopen(nomeArquivo, "w+");

    if (arquivo == NULL) {
        fprintf(stderr, "Nao foi possivel abrir o aquivo %s\n", nomeArquivo);
        exit(EXIT_FAILURE);
    }

    return arquivo;
}

FILE *abreArquivoAtualizacao(char *nomeArquivo) {
    /*
        Ambos r+e w+podem ler e gravar em um arquivo. No entanto, r+ não exclui o conteúdo do
        arquivo e não cria um novo arquivo se tal arquivo não existir, enquanto w+ exclui o 
        conteúdo do arquivo e o cria se ele não existir.

        URL: https://stackoverflow.com/questions/21113919/difference-between-r-and-w-in-fopen
    */
    FILE *arquivo = fopen(nomeArquivo, "r+");

    if (arquivo == NULL) {
        fprintf(stderr, "Nao foi possivel abrir o aquivo %s\n", nomeArquivo);
        exit(EXIT_FAILURE);
    }

    return arquivo;
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