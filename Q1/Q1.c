#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Os monitores instruiram a usar um define para os valores de N e X que podem ser alterados posteriormente*/
/*O arquivo teste tem essa configuracao, mas pode ser alterado*/
#define n 5
#define x 10

/*Estrutura das Threads*/
typedef struct
{
    int thread_id;
    char **files;
    int file_count;
    char *word;
} ThreadData;

/*Funcao que faz a procura da palavra nos arquivos*/
void *search_in_files(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    char *result = (char *)malloc(1000 * data->file_count);
    strcpy(result, "");

    for (int i = 0; i < data->file_count; i++)
    {
        FILE *file = fopen(data->files[i], "r");
        if (file == NULL)
        {
            printf("Erro ao abrir o arquivo %s\n", data->files[i]);
            continue;
        }

        char line[1024];
        int line_number = 0;

        while (fgets(line, sizeof(line), file))
        {
            line_number++;
            char *occurrence_pos = line;
            while ((occurrence_pos = strstr(occurrence_pos, data->word)) != NULL)
            {
                char occurrence[256];
                sprintf(occurrence, "<%s>:<%d>\n", data->files[i], line_number);
                strcat(result, occurrence);
                occurrence_pos++;
            }
        }
        fclose(file);
    }
    pthread_exit((void *)result);
}

int main(int argc, char *argv[])
{
    /*Inicializando as Threads*/
    pthread_t threads[n];
    ThreadData thread_data[n];

    /*Nomes dos arquivos (O arquivo exemplo possuira 10 ".txt")*/
    char **files = (char **)malloc(x * sizeof(char *));
    for (int i = 0; i < x; i++)
    {
        files[i] = (char *)malloc(10 * sizeof(char));
        sprintf(files[i], "%d.txt", i + 1);
    }

    /*Distribuindo os arquivos entre as threads*/
    int files_per_thread = x / n;
    int remaining_files = x % n;

    /*Para inserir a palavra que sera buscada*/
    /*Alguns exemplos de palavras que aparecem nos arquivos, que podem ser usadas: "linguagem", "Estruturas", "Git"*/
    char word[1000];
    printf("Digite a palavra a ser buscada: ");
    scanf("%s", word);

    /*Criando as threads e dividindo os arquivos entre as threads*/
    /*Alem disso eh chamada a funcao que realiza a busca*/
    int current_file = 0;
    for (int t = 0; t < n; t++)
    {
        int thread_file_count = files_per_thread + (t < remaining_files ? 1 : 0);
        thread_data[t].files = &files[current_file];
        thread_data[t].file_count = thread_file_count;
        thread_data[t].word = word;
        thread_data[t].thread_id = t;

        current_file += thread_file_count;

        pthread_create(&threads[t], NULL, search_in_files, (void *)&thread_data[t]);
    }

    /*Mostrando os resultados de cada thread*/
    for (int u = 0; u < n; u++)
    {
        char *res;
        pthread_join(threads[u], (void **)&res);
        printf("%s", res);
        free(res);
    }
    
    return 0;
}
