#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Definindo a variavel n como sendo 5 mas pode ser qualquer valor inteiro positivo*/
#define n 5
/*Definindo um tamanho para o buffer mas tambem pode ser qualquer valor inteiro positivo*/
/*Se quiser aumentar o numero de requisicoes simuladas basta aumentar essa variavel*/
#define TAM_BUFFER 10

/*Struct da Requisicao*/
typedef struct {
    void *(*funexec)(void *);
    void *arg;
    int id;
} Requisicao;

/*Struct do Resultado*/
typedef struct {
    int id;
    void *resultado;
    int esta_disponivel;
} Resultado;

/*Buffer de requisicoes*/
Requisicao bufferRequisicao[TAM_BUFFER];
int comecoBuffer = 0, fimBuffer = 0, contadorBuffer = 0;

/*Buffer que armazena os resultados*/
Resultado bufferResultado[TAM_BUFFER];

/*Inicializando mutexes e variaveis de condicao*/
pthread_mutex_t resultadoMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t requisicaoDisponivel = PTHREAD_COND_INITIALIZER;
pthread_cond_t espacoDisponivel = PTHREAD_COND_INITIALIZER;
pthread_cond_t resultadoPronto = PTHREAD_COND_INITIALIZER;

/*Variaveis globais*/
int proxID = 0;
int continuarExecutando = 1;

/*Thread Despachante*/
void *despachante(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&bufferMutex);

        /*Espera enquanto nao tem requisicoes no buffer*/
        while(contadorBuffer == 0 && continuarExecutando)
        {
            pthread_cond_wait(&requisicaoDisponivel, &bufferMutex);
        }

        /*Confere se eh pra continuar executando*/
        if(!continuarExecutando && contadorBuffer == 0)
        {
            pthread_mutex_unlock(&bufferMutex);
            break;
        }

        /*Retira a proxima requisicao do buffer*/
        Requisicao req = bufferRequisicao[comecoBuffer];
        comecoBuffer = (comecoBuffer + 1) % TAM_BUFFER;
        contadorBuffer--;
        pthread_cond_signal(&espacoDisponivel);

        pthread_mutex_unlock(&bufferMutex);

        void *resultado = req.funexec(req.arg);

        pthread_mutex_lock(&resultadoMutex);

        bufferResultado[req.id].id = req.id;
        bufferResultado[req.id].resultado = resultado;
        bufferResultado[req.id].esta_disponivel = 1;

        pthread_cond_broadcast(&resultadoPronto);
        pthread_mutex_unlock(&resultadoMutex);
    }
    return NULL;
}

/*Funcao para agendar a execucao de funexec para uma requisicao*/
int agendarExecucao(void *(*funexec)(void *), void *arg)
{
    pthread_mutex_lock(&bufferMutex);

    while (contadorBuffer == TAM_BUFFER) {
        pthread_cond_wait(&espacoDisponivel, &bufferMutex);
    }

    /*Gera o ID*/
    int id = proxID++ % TAM_BUFFER;

    /*Cria a requisicao e insere no buffer*/
    bufferRequisicao[fimBuffer].funexec = funexec;
    bufferRequisicao[fimBuffer].arg = arg;
    bufferRequisicao[fimBuffer].id = id;
    fimBuffer = (fimBuffer + 1) % TAM_BUFFER;
    contadorBuffer++;

    /*Inicializa o buffer de resultados para esse ID*/
    bufferResultado[id].id = id;
    bufferResultado[id].esta_disponivel = 0;

    /*Sinaliza que ha requisicoes disponiveis para a thread Despachante*/
    pthread_cond_signal(&requisicaoDisponivel);

    pthread_mutex_unlock(&bufferMutex);

    return id;
}

/*Funcao para pegar o resultado de uma requisição*/
void *pegarResultadoExecucao(int id)
{
    pthread_mutex_lock(&resultadoMutex);

    while(!bufferResultado[id].esta_disponivel)
    {
        pthread_cond_wait(&resultadoPronto, &resultadoMutex);
    }

    void *resultado = bufferResultado[id].resultado;
    pthread_mutex_unlock(&resultadoMutex);

    return resultado;
}

/*Funcao exemplo que sera executada quando for feita uma requisicao*/
/*Ela pega o arg e transforma em inteiro e retorna isso como resultado*/
/*Como ela e uma funcao exemplo ela so faz isso mas pode ser implementado para ela fazer mais algo*/
void *funexec(void *arg)
{
    int *num = (int *)arg;
    int *resultado = malloc(sizeof(int));
    *resultado = (*num);
    printf("Função executada com argumento %d, resultado: %d\n", *num, *resultado);
    return resultado;
}

int main(int argc, char *argv[])
{
    /*Criando a Thread Despachante*/
    pthread_t thread_despachante;
    pthread_create(&thread_despachante, NULL, despachante, NULL);

    /*Simulando Requisições*/
    for (int i = 0; i < TAM_BUFFER; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        int id = agendarExecucao(funexec, arg);
        printf("Função agendada com ID %d\n", id);
    }

    /*Pegando Resultados*/
    for(int i = 0; i < TAM_BUFFER; i++)
    {
        int *resultado = (int *)pegarResultadoExecucao(i);
        printf("Resultado da requisição %d: %d\n", i, *resultado);
        free(resultado);
    }

    /*Para a despachante*/
    pthread_mutex_lock(&bufferMutex);
    continuarExecutando = 0;
    pthread_cond_signal(&requisicaoDisponivel);
    pthread_mutex_unlock(&bufferMutex);
    pthread_join(thread_despachante, NULL);

    /*Libera os recursos associados aos mutexes e variaveis de condicao*/
    pthread_mutex_destroy(&resultadoMutex);
    pthread_mutex_destroy(&bufferMutex);
    pthread_cond_destroy(&espacoDisponivel);
    pthread_cond_destroy(&requisicaoDisponivel);
    pthread_cond_destroy(&resultadoPronto);

    return 0;
}
