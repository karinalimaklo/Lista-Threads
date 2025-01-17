#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Definição do que será necessário, como tamanho e a matriz do tabuleiro.
#define TAM 3
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char tabuleiro[TAM][TAM];
int resultado = -1;

//Como a matriz é dada, o tabuleiro está num arquivo que será lido pelo programa.
//Função para a leitura do arquivo.
void ler_arquivo(const char *arquivo){

    FILE *file = fopen(arquivo, "r");

    //Se o arquivo estiver vazio, retorna uma mensagem de erro.
    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < TAM; i++)
    {
        for (int j = 0; j < TAM; j++)
        {
            fscanf(file, " %c", &tabuleiro[i][j]); //Lê um caractere do arquivo e o armazena na posição tabuleiro[i][j]
        }
    }
    fclose(file);
    
}

//Funções para fazer a verificação de se as linhas/colunas/diagonais possuem a mesma marca.
int verificar_linhas (int linha) {
    return tabuleiro[linha][0] == tabuleiro[linha][1] && tabuleiro[linha][1] == tabuleiro[linha][2];
}

int verificar_colunas (int coluna) {
    return tabuleiro[0][coluna] == tabuleiro[1][coluna] && tabuleiro[1][coluna] == tabuleiro[2][coluna];
}

int verificar_diagprincipal() {
    return tabuleiro[0][0] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][2];
}

int verificar_diagsecundaria() {
    return tabuleiro[0][2] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][0];
}


//Funções responsáveis por verificar as linhas, as colunas e as diagonais.
void *linhas (void *thread_data) {
    
    for(int i = 0; i < TAM; i++)
    {
        //Se uma linha for toda igual, um dos jogadores ganhou, portanto alteramos o valor do resultado.
        if (verificar_linhas(i))
        {
            if(tabuleiro[i][0] == 'O') {
                pthread_mutex_lock(&mutex); //Trava o mutex
                resultado = 0;
                pthread_mutex_unlock(&mutex); //Destrava o mutex
            } else if(tabuleiro[i][0] == 'X') {
                pthread_mutex_lock(&mutex);
                resultado = 1;
                pthread_mutex_unlock(&mutex);
            }
        }
    }
    pthread_exit(NULL);

}

void *colunas (void *thread_data) {

    for(int i = 0; i < TAM; i++)
    {
        //Como anteriormente, verificamos se em uma coluna todas as marcas são iguais
        if (verificar_colunas(i))
        {
            if(tabuleiro[0][i] == 'O') {
                pthread_mutex_lock(&mutex); //Trava o mutex
                resultado = 0;
                pthread_mutex_unlock(&mutex); //Destrava o mutex
            } else if(tabuleiro[0][i] == 'X') {
                pthread_mutex_lock(&mutex);
                resultado = 1;
                pthread_mutex_unlock(&mutex);
            }
        }
    }
    pthread_exit(NULL);

}

void *diagonais (void *thread_data) {

    //Verificamos se as marcas da diagonal são iguais.
    if (verificar_diagprincipal())
    {
        if(tabuleiro[0][0] == 'O') {
            pthread_mutex_lock(&mutex); //Trava o mutex
            resultado = 0;
            pthread_mutex_unlock(&mutex); //Destrava o mutex
        } else if(tabuleiro[0][0] == 'X') {
            pthread_mutex_lock(&mutex);
            resultado = 1;
            pthread_mutex_unlock(&mutex);
        }
    }

    if (verificar_diagsecundaria())
    {
        if (tabuleiro[0][2] == 'O') {
            pthread_mutex_lock(&mutex); //Trava o mutex
            resultado = 0;
            pthread_mutex_unlock(&mutex); //Destrava o mutex
        } else if (tabuleiro[0][2] == 'X') {
            pthread_mutex_lock(&mutex);
            resultado = 1;
            pthread_mutex_unlock(&mutex);
        }
        
    }
    

    pthread_exit(NULL);
}


//Função para imprimir o tabuleiro para facilitar a visualização.
void imprimir (){
    
    for (int i = 0; i < TAM; i++)
    {
        for (int j = 0; j < TAM; j++) {
            printf("%c ", tabuleiro[i][j]);
        }
        printf("\n");
    }
    
}


int main (int argc, char *argv[]) {

    //Chamamos as funções de ler o arquivo, imprimir o tabuleiro e declaramos 3 threads
    ler_arquivo(argv[1]);
    imprimir();
    pthread_t thread[3];


    //Criação das threads.
    pthread_create(&thread[0], NULL, linhas, NULL);
    pthread_create(&thread[1], NULL, colunas, NULL);
    pthread_create(&thread[2], NULL, diagonais, NULL);

    for (int i = 0; i < 3; i++)
    {
        pthread_join(thread[i], NULL); //Espera que as threads terminem.
    }

    //De acordo com o resultado, declaramos o vencedor.
    if (resultado == 0)
    {
        printf("O jogador 1 venceu!\n");
    } else if (resultado == 1) {
        printf("O jogador 2 venceu!\n");
    } else {
        printf("Deu velha!\n");
    }
    
    return 0;

}
