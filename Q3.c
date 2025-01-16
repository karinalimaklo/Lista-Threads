#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// A questão não falou exatamente os valores de N e T, então resolvi usar defines, para poder alterar os valores facilmente
#define TOTAL_CARROS_LADO1 12
#define TOTAL_CARROS_LADO2 16
#define MAX_CARROS_NA_PONTE 3
#define MAX_CARROS_CONSECUTIVOS 5
// o enunciado pediu para implementar um mecanismo que priorizasse os veículos de ambas as direções de forma justa, 
// então eu implementei uma solução em que há uma troca de prioridade entre os lados após um certo número de carros atravessarem a ponte,
// o que garante que carros de ambos os lados possam passar sem que uma direção fique travada indefinidamente.

//utilizei mutex e variáveis de condição para evitar condição de disputa
pthread_mutex_t mutex;
pthread_cond_t cond_carro1;
pthread_cond_t cond_carro2;

//variáveis globais
int carros_na_ponte = 0;
int carros_consecutivos = 0;
int direcao_atual = 0; // 0 = nenhum, 1 = lado 1, 2 = lado 2
int carros_restantes_lado1 = TOTAL_CARROS_LADO1;
int carros_restantes_lado2 = TOTAL_CARROS_LADO2;

void *carro_lado1(void *threadid) {
    int id = *((int *)threadid);
    free(threadid);
    // aloquuei memória na função main e após copiar o valor necessário para a variável id, essa memória não é mais necessária, então já dei free

    pthread_mutex_lock(&mutex);

    //se alguma dessas condições for atendida, os carros do lado 1 esperam pela liberação da ponte
    while (((direcao_atual == 2 && carros_restantes_lado2 > 0) || carros_na_ponte >= MAX_CARROS_NA_PONTE ||
           (carros_consecutivos >= MAX_CARROS_CONSECUTIVOS && carros_restantes_lado2 > 0))) {
        pthread_cond_wait(&cond_carro1, &mutex);
    }

    //carros do lado 1 estão atravessando a ponte
    carros_na_ponte++;
    carros_consecutivos++;
    direcao_atual = 1;

    //utilizei esses printfs para checar se os carros realmente estavam subindo e descendo da ponte, mas como a questão não pediu nenhuma saída, deixei eles comentados 
    //printf("Carro %d do lado 1 está na ponte. Carros na ponte: %d\n", id, carros_na_ponte);
    pthread_mutex_unlock(&mutex);

    // Utilizei sleep para simular o tempo de travessia da ponte
    sleep(1);

    pthread_mutex_lock(&mutex);

    // Quando a ponte está com capacidade máxima, essa condição é executada para que o carro "da frente" saia e o próximo possa subir
    if(carros_na_ponte>= MAX_CARROS_NA_PONTE){
        carros_na_ponte--;
        carros_restantes_lado1--;
        //printf("Carro %d do lado 1 saiu da ponte. Carros na ponte: %d\n", id, carros_na_ponte);
    }
    // Aqui, temos condições para poder alterar a direção que terá prioridade, 
    // dependendo se a capacidade máxima da ponte é maior ou menor que a quantidade de carros consecutivos vindo de uma certa direção 
    if(MAX_CARROS_NA_PONTE <= MAX_CARROS_CONSECUTIVOS){
        if ((carros_consecutivos >= MAX_CARROS_CONSECUTIVOS && carros_na_ponte == (MAX_CARROS_NA_PONTE - 1) && carros_restantes_lado1 > 0) || (carros_restantes_lado1 == 0)) {
            // se as condições são atendidas, agora é a vez dos carros da direção 2 atravessarem a ponte
            direcao_atual = 2;
            carros_consecutivos = 0;  // zeramos a contagem de carros consecutivos para recomeçar a contar para os carros vindos do lado 2 
            pthread_cond_broadcast(&cond_carro2); 
        }
        else{ 
            //se as condições não são atendidas, os carros que vem da direção 1 continuam a atravessar a ponte 
            pthread_cond_broadcast(&cond_carro1);
        }
    }
    else{ 
        if((carros_consecutivos >= MAX_CARROS_CONSECUTIVOS && carros_na_ponte > 0 && carros_restantes_lado1 > 0) || (carros_restantes_lado1 == 0)){
            direcao_atual = 2;
            carros_consecutivos = 0;
            pthread_cond_broadcast(&cond_carro2);
        }
        else{
            pthread_cond_broadcast(&cond_carro1);
        }
    }

    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}
// similar a função de cima, mas para os carros vindos da direção 2
void *carro_lado2(void *threadid) {
    int id = *((int *)threadid);
    free(threadid);

    pthread_mutex_lock(&mutex);

   while (((direcao_atual == 1 && carros_restantes_lado1 > 0) || carros_na_ponte >= MAX_CARROS_NA_PONTE ||
           (carros_consecutivos >= MAX_CARROS_CONSECUTIVOS && carros_restantes_lado1 > 0))) {
        pthread_cond_wait(&cond_carro2, &mutex);
    }

    carros_na_ponte++;
    carros_consecutivos++;
    direcao_atual = 2;

    //printf("Carro %d do lado 2 está na ponte. Carros na ponte: %d\n", id, carros_na_ponte);
    pthread_mutex_unlock(&mutex);
    sleep(1);
    pthread_mutex_lock(&mutex);
    
     if(carros_na_ponte>= MAX_CARROS_NA_PONTE){
        carros_na_ponte--;
        carros_restantes_lado2--;
        //printf("Carro %d do lado 2 saiu da ponte. Carros na ponte: %d\n", id, carros_na_ponte);
    }

   if(MAX_CARROS_NA_PONTE <= MAX_CARROS_CONSECUTIVOS){
        if ((carros_consecutivos >= MAX_CARROS_CONSECUTIVOS && carros_na_ponte == (MAX_CARROS_NA_PONTE - 1) && carros_restantes_lado2 > 0) || (carros_restantes_lado2 == 0)) {
            direcao_atual = 1;
            carros_consecutivos = 0;
            pthread_cond_broadcast(&cond_carro1);
        }
        else{
            pthread_cond_broadcast(&cond_carro2);
        }
    }
    else{
        if((carros_consecutivos >= MAX_CARROS_CONSECUTIVOS && carros_na_ponte > 0 && carros_restantes_lado2 > 0) || (carros_restantes_lado2 == 0)){
            direcao_atual = 1;
            carros_consecutivos = 0;
            pthread_cond_broadcast(&cond_carro1);
        }
        else{
            pthread_cond_broadcast(&cond_carro2);
        }
    }

    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main() {
    // vetores de threads para representar os carros de cada lado
    pthread_t threads_lado1[TOTAL_CARROS_LADO1];
    pthread_t threads_lado2[TOTAL_CARROS_LADO2];

    // inicializamos o mutex e as variáveis de condição
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_carro1, NULL);
    pthread_cond_init(&cond_carro2, NULL);

    // criação das threads para o lado 1 e lado 2 
    for (int i = 0; i < TOTAL_CARROS_LADO1; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads_lado1[i], NULL, carro_lado1, id);
    }

    for (int i = 0; i < TOTAL_CARROS_LADO2; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads_lado2[i], NULL, carro_lado2, id);
    }
    // join para garantir que a main espere que todas as threads dos carros terminem antes de prosseguir sua execução
    for (int i = 0; i < TOTAL_CARROS_LADO1; i++) {
        pthread_join(threads_lado1[i], NULL);
    }

    for (int i = 0; i < TOTAL_CARROS_LADO2; i++) {
        pthread_join(threads_lado2[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_carro1);
    pthread_cond_destroy(&cond_carro2);

    return 0;
}