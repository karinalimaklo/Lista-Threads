#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Definição do que será necessário, como tamanho e a matriz zerada.
#define TAM 3
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char tabuleiro[TAM][TAM] = { 
    {' ', ' ', ' '},
    {' ', ' ', ' '},
    {' ', ' ', ' '}
    
};

int resultado = -1;

//Funções responsáveis por verificar as linhas, as colunas e as diagonais.
void *linhas (void *thread_data) {
    
    for(int i = 0; i < TAM; i++)
    {
        //Se uma linha for toda igual, um dos jogadores ganhou, portanto alteramos o valor do resultado.
        if (tabuleiro[i][0] == tabuleiro[i][1] && tabuleiro[i][1] == tabuleiro[i][2])
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
        if (tabuleiro[0][i] == tabuleiro[1][i] && tabuleiro[1][i] == tabuleiro[2][i])
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
    if (tabuleiro[0][0] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][2])
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

    pthread_exit(NULL);
}

//Função para coletar os dados do teclado.
int coletar_dados (int player) {

    int linha, coluna;
    char marca;
    if(player == 1){marca = 'O';}else{marca = 'X';}

    printf("Jogador %d (%c): Digite a linha e a coluna desejada.\n", player, marca);
    scanf("%d %d", &linha, &coluna);

    //Verificamos se a linha e a coluna desejada está livre.
    if(tabuleiro[linha][coluna] == ' ' && linha >= 0 && linha < TAM && coluna >= 0 && coluna < TAM)
    {
        tabuleiro[linha][coluna] = marca;
    }else{
        printf("Posicao invalida. Tente novamente!\n");
    }

}

//Função para imprimir o tabuleiro para facilitar a visualização de quais posições ainda estão livres.
void imprimir (){
    
    for (int i = 0; i < TAM; i++)
    {
        for (int j = 0; j < TAM; j++) {
            printf("%c ", tabuleiro[i][j]);
        }
        printf("\n");
    }
    
}

//Função para verificar se houve um empate, ou seja, deu velha.
int empatou () {

    //Se ele retornar 1, significa que não há espaços vazios e o tabuleiro foi totalmente preenchido.
    for (int i = 0; i < TAM; i++)
    {
        for (int j = 0; j < TAM; j++)
        {
            if (tabuleiro[i][j] == ' ')
            {
                return 0;
            }
        }
    }

        return 1;
}

int main (int argc, char *argv[]) {

    //Começamos com o jogador 1 e declaramos 3 threads, uma para cada função.
    int player = 1;
    pthread_t thread[3];

    //Enquanto o resultado não foi definido, o loop será executado.
    while (resultado == -1) {
        coletar_dados(player);
        imprimir ();

        //Criação das threads.
        pthread_create(&thread[0], NULL, linhas, NULL);
        pthread_create(&thread[1], NULL, colunas, NULL);
        pthread_create(&thread[2], NULL, diagonais, NULL);

        for (int i = 0; i < 3; i++)
        {
            pthread_join(thread[i], NULL); //Espera que as threads terminem.
        }

        if (player == 1){player = 2;}else{player = 1;}

        if(empatou()){
            printf("Deu velha!\n");
            break;
        }
      
    }

    //De acordo com o resultado, declaramos o vencedor.
    if (resultado == 0)
    {
        printf("O jogador 1 venceu!\n");
    } else if (resultado == 1) {
        printf("O jogador 2 venceu!\n");
    }
    
    return 0;

}