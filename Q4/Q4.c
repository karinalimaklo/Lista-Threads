#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h> //biblioteca pra usar sleep

#define P 5
#define C 7
#define B 8

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; //sinaliza se a fila está vazia
pthread_cond_t full = PTHREAD_COND_INITIALIZER; //sinaliza se a fila está cheia

typedef struct elem {
    int value;
    struct elem *prox;
} Elem;

typedef struct blockingQueue {
    unsigned int sizeBuffer, statusBuffer;
    Elem *head, *last;
} BlockingQueue;

BlockingQueue* Q;

//cria uma nova fila bloqueante do tamanho do valor passado.
BlockingQueue* newBlockingQueue(unsigned int sizeBuffer) {

    //alocando memória para um nó
    Elem* no = (Elem*) malloc(sizeof(Elem)); //alocando memória para um nó
    no->prox = NULL;

    //inicializa a fila bloqueante
    BlockingQueue* fila = (BlockingQueue*) malloc(sizeof(BlockingQueue));
    fila->sizeBuffer = sizeBuffer;
    fila->statusBuffer = 0; //começa vazia
    fila->head = no;
    fila->last = no;
    return fila;
}

//insere um elemento no final da fila bloqueante Q, bloqueando a thread que está inserindo, caso a fila esteja cheia.
void putBlockingQueue(BlockingQueue* Q, int newValue) {
    
    pthread_mutex_lock(&mutex); //trava o mutex
    //enquanto a fila estiver cheia, bloqueia a thread que está inserindo
    while (Q->statusBuffer == Q->sizeBuffer) {
        printf("A fila está cheia, as threads produtoras estão dormindo!\n");
        pthread_cond_wait(&empty, &mutex);
    }

    //cria um novo elemento 
    Elem* novoElem = (Elem*) malloc(sizeof(Elem));
    novoElem->value = newValue;
    novoElem->prox = NULL;
    //insere o novo elemento no final da fila
    Q->last->prox = novoElem;
    Q->last = novoElem;
    Q->statusBuffer++;

    //sinalizar para as threads consumidoras que há um novo item e destravar o mutex
    pthread_cond_broadcast(&full);
    pthread_mutex_unlock(&mutex); 
}

//retira o primeiro elemento da fila bloqueante Q, bloqueando a thread que está retirando, caso a fila esteja vazia.
int takeBlockingQueue(BlockingQueue* Q) {
    pthread_mutex_lock(&mutex); //trava o mutex

    //espera enquanto a fila está vazia
    while (Q->statusBuffer == 0) {
        printf("A fila está vazia, as threads consumidoras estão dormindo!\n");
        pthread_cond_wait(&full, &mutex);
    }

    //retira o primeiro elemento da fila
    Elem* temp = Q->head->prox;
    Q->head->prox = temp->prox;
    int valorRetirado = temp->value;
    free(temp);
    Q->statusBuffer--;

    //se a fila ficar vazia, ajusta os ponteiros
    if (Q->statusBuffer == 0) {
        Q->last = Q->head;
    }

    //sinalizar para as threads produtoras que um item foi retirado e destravar o mutex
    pthread_cond_broadcast(&empty);
    pthread_mutex_unlock(&mutex); 
    return valorRetirado;
}

//função executada pelas threads produtoras
void* produtor(void* threadid) {
    int id = *((int*) threadid);
    while (1) {
        int valor = rand() % 10; //gera um valor aleatório
        putBlockingQueue(Q, valor);
        printf("Produtor %d produziu o valor %d\n", id, valor);
        sleep(1); //atrasa a execução da thread para evitar loops rápidos
    }
    pthread_exit(NULL);
}

//função executada pelas threads consumidoras
void* consumidor(void* threadid) {
    int id = *((int*) threadid);
    while (1) {
        int valor = takeBlockingQueue(Q);
        printf("Consumidor %d consumiu o valor %d\n", id, valor);
        sleep(1); 
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    
    pthread_t threads_produtoras[P];
    pthread_t threads_consumidoras[C];
    int* idProd[P];
    int* idCons[C];

    Q = newBlockingQueue(B);

    //criação das threads produtoras e consumidoras
    for (int i = 0; i < P; i++) {
        idProd[i] = malloc(sizeof(int));
        *idProd[i] = i + 1;
        pthread_create(&threads_produtoras[i], NULL, produtor, (void*) idProd[i]);
    }

    for (int i = 0; i < C; i++) {
        idCons[i] = malloc(sizeof(int));
        *idCons[i] = i + 1;
        pthread_create(&threads_consumidoras[i], NULL, consumidor, (void*) idCons[i]);
    }

    //aguarda as threads produtoras e consumidoras terminarem
    for (int i = 0; i < P; i++) {
        pthread_join(threads_produtoras[i], NULL);
    }
    for (int i = 0; i < C; i++) {
        pthread_join(threads_consumidoras[i], NULL);
    }

    return 0;
}