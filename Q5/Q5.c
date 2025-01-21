#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Definindo a variavel n como sendo 5 mas pode ser qualquer valor inteiro positivo*/
#define n 5
/*Definindo um tamanho para o buffer mas tambem pode ser qualquer valor inteiro positivo*/
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

/*Mutexes e variaveis de condicao para evitar condicao de disputa*/
pthread_mutex_t resultadoMutex;
pthread_mutex_t bufferMutex;
pthread_mutex_t contadorThreadMutex;
pthread_cond_t requisicaoDisponivel;
pthread_cond_t resultadoPronto;

/*Variaveis globais*/
int threadsAtivas = 0;
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
        if(!continuarExecutando)
        {
            pthread_mutex_unlock(&bufferMutex);
            break;
        }

        /*Retira a proxima requisicao do buffer*/
        Requisicao req = bufferRequisicao[comecoBuffer];
        comecoBuffer = (comecoBuffer + 1) % TAM_BUFFER;
        contadorBuffer--;

        pthread_mutex_unlock(&bufferMutex);

        /*ANALISAR OUTRA FORMA ACHO QUE COM VARIAVEL DE CONDICAO*/
        pthread_mutex_lock(&contadorThreadMutex);
        while(threadsAtivas >= n)
        {
            pthread_mutex_unlock(&contadorThreadMutex);
            pthread_mutex_lock(&contadorThreadMutex);
        }
        threadsAtivas++;
        pthread_mutex_unlock(&contadorThreadMutex);

        /*Cria uma nova thread para executar a requisicao*/
        pthread_t threadReq;
        pthread_create(&threadReq, NULL, req.funexec, req.arg);

        /*Aguarda a thread terminar e salva o resultado*/
        void *resultado;
        pthread_join(threadReq, &resultado);

        pthread_mutex_lock(&resultadoMutex);
        bufferResultado[req.id].id = req.id;
        bufferResultado[req.id].resultado = resultado;
        bufferResultado[req.id].esta_disponivel = 1;
        pthread_mutex_unlock(&resultadoMutex);

        /*Atualiza o numero de threads*/
        pthread_mutex_lock(&contadorThreadMutex);
        threadsAtivas--;
        pthread_mutex_unlock(&contadorThreadMutex);

        /*Sinaliza que o resultado esta disponivel*/
        pthread_cond_broadcast(&resultadoPronto);
    }
    return NULL;
}

/*Funcao para agendar a execucao de uma requisicao*/
int agendarExecucao(void *(*funexec)(void *), void *arg)
{
    pthread_mutex_lock(&bufferMutex);

    /*Gera o ID*/
    int id = proxID++ % TAM_BUFFER;

    /*Cria a requisicao*/
    Requisicao req;
    req.funexec = funexec;
    req.arg = arg;
    req.id = id;

    /*Insere a requisicao no buffer*/
    bufferRequisicao[fimBuffer] = req;
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

    int *resultado = (int *)bufferResultado[id].resultado;
    printf("Resultado do ID %d: %d\n", id, *resultado);
    free(resultado);

    pthread_mutex_unlock(&resultadoMutex);
}

/*o struct deverá ser passado como argumento para funexec durante a criação da thread.*/
/*Qual struct?*/
void *funexec(void *arg)
{
    int *num = (int *)arg;
    int *resultado = malloc(sizeof(int));
    *resultado = (*num) * (*num); /*Porque calcular o quadrado?*/
    printf("Função executada com argumento %d, resultado: %d\n", *num, *resultado);
    return resultado;
}

int main(int argc, char *argv[])
{
    /*Criando a Thread Despachante*/
    pthread_t thread_despachante;
    pthread_create(&thread_despachante, NULL, despachante, NULL);

    /*Inicializando mutexes e variaveis de condicao*/
    pthread_mutex_init(&resultadoMutex, NULL);
    pthread_mutex_init(&bufferMutex, NULL);
    pthread_mutex_init(&contadorThreadMutex, NULL);
    pthread_cond_init(&requisicaoDisponivel, NULL);
    pthread_cond_init(&resultadoPronto, NULL);

    /*Simulando Requisições*/
    for (int i = 0; i < 10; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        int id = agendarExecucao(funexec, arg);
        printf("Função agendada com ID %d\n", id);
    }

    /*Pegando Resultados*/
    for(int i = 0; i < 10; i++)
    {
        pegarResultadoExecucao(i);
    }

    /*Para a despachante*/
    continuarExecutando = 0;
    pthread_cond_signal(&requisicaoDisponivel);
    pthread_join(thread_despachante, NULL);

    /*Libera os recursos associados aos mutexes e variaveis de condicao*/
    pthread_mutex_destroy(&resultadoMutex);
    pthread_mutex_destroy(&bufferMutex);
    pthread_mutex_destroy(&contadorThreadMutex);
    pthread_cond_destroy(&requisicaoDisponivel);
    pthread_cond_destroy(&resultadoPronto);

    return 0;
}