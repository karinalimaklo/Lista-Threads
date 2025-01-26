#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N 5
#define M 5
#define TAM_ARRAY 20

pthread_mutex_t oMutex = PTHREAD_MUTEX_INITIALIZER; // Para nao dar problema na região critica
pthread_cond_t podeLer = PTHREAD_COND_INITIALIZER; //Para fazer as threads leitora executarem quando puder
pthread_cond_t podeEscrever = PTHREAD_COND_INITIALIZER; //Para fazer as escritoras executarem

int BaseComp[TAM_ARRAY]; //Array com os dados
int CountLeiRC=0; //Diz quantos leitores estão ativamente lendo

void *Leitoras(void *threadid);
void *Escritoras(void *threadid);

int main (int argc, char *argv[]){

    for(int i=0; i<TAM_ARRAY; i++){
        BaseComp[i]=0;
    }

    //Vetores e ponteiros para as threads e seus IDs
    pthread_t threadsLeitores[N];
    int *leitoresID[N];
    pthread_t threadsEscritores[M];
    int *escritoresID[M];

    for(int i=0; i<N; i++){ //Cria as threads leitoras
        leitoresID[i]=(int *)malloc(sizeof(int));
        *leitoresID[i]=i;
        pthread_create(&threadsLeitores[i], NULL, Leitoras, leitoresID[i]);
    }
    for(int i=0; i<N; i++){ //Cria as threads escritoras
        escritoresID[i] = (int *)malloc(sizeof(int));
        *escritoresID[i]=i;
        pthread_create(&threadsEscritores[i], NULL, Escritoras, escritoresID[i]);
    }

    for(int i=0; i<N+M; i++){ //Para que todas possam terminar, nesse caso nunca param (laço infinito)
        if(i<N){
            pthread_join(threadsLeitores[i], NULL);
        } else{
            pthread_join(threadsEscritores[i-N], NULL);
        }
    }

    pthread_mutex_destroy(&oMutex);
    pthread_cond_destroy(&podeLer);
    pthread_cond_destroy(&podeEscrever);

    pthread_exit(NULL); //Acaba o programa
    return 0;
}

void *Leitoras(void *threadid){
    int tid = *((int *)threadid); //Para pegar o ID da thread

    while(1){

        pthread_mutex_lock(&oMutex); //Bloqueia a região crítica
        while(CountLeiRC == -1){ //Só fica -1 quando alguma está escrevendo
            pthread_cond_wait(&podeLer, &oMutex); //Verifica se pode executar, se não puder fica inativa até que seja chamada
        }
        CountLeiRC++;
        pthread_mutex_unlock(&oMutex); //Libera a RC

        int num=rand()%TAM_ARRAY;
        printf("O número da posição %d é %d, segundo a leitora %d\n", num, BaseComp[num], tid); //Diz o número de alguma posição aleatória

        pthread_mutex_lock(&oMutex);
        CountLeiRC--; //Parou de ler, então diminui os leitores na RC
        if(CountLeiRC==0){
            pthread_cond_signal(&podeEscrever); //Manda sinal para alguma leitora poder executar
        }
        pthread_mutex_unlock(&oMutex);
        sleep(2); //Pausa de tempo
    }

    return 0;
}
void *Escritoras(void *threadid){
    int tid = *((int *)threadid);

    while(1){

        pthread_mutex_lock(&oMutex);
        while(CountLeiRC!=0){ //So entra se alguma leitora estiver ativa (algum leitor na RC)
            pthread_cond_wait(&podeEscrever, &oMutex);
        }
        CountLeiRC=-1;
        pthread_mutex_unlock(&oMutex);

        int num=rand()%TAM_ARRAY;
        int dado=rand();
        BaseComp[num]=dado;
        printf("A posição %d foi preenchida com %d pela escritora %d\n", num, BaseComp[num], tid); //Preenche uma posição aleátoria com um dado qualquer

        pthread_mutex_lock(&oMutex); //Para atualizar o contador sem problemas e chamar outras threads
        CountLeiRC=0;
        pthread_cond_broadcast(&podeLer);
        pthread_cond_signal(&podeEscrever);
        pthread_mutex_unlock(&oMutex);
        sleep(1);
    }

    return 0;
}
