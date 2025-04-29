#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

double get_time();
int powerof2(int);
void* parallel_qs(void*);
void serial_qs(int *, int, int);

typedef struct {
    int myid, N, NT;
    int * arr;
    int ** local_arr; //global view of local arrays
    int * local_arr_size;
    
} thread_arg_t;

int main(int ac, char** av) {
    if (ac != 6) {
        printf("Usage: ./%s N input output NT S\n", av[0]);
        printf("N - Number of Elements to Sort\n");
        printf("input - Input Numbers Dataset Filepath (generate with gen_data.c)\n");
        printf("output - Filepath to store the sorted elements\n");
        printf("NT - Number of Processors to Utilise\n");
        printf("strat - Strategy to Select the pivot element:\n");
        printf("\t\'a\' - Pick Median of Processor-0 in Processor Group\n");
        printf("\t\'b\' - Select the mean of all medians in respective processor set\n");
        printf("\t\'c\' - Sort the medians and select the mean value of the two middlemost medians in each processor set\n");

        return 1;
    }
    int N = atoi(av[1]);
    char *inputfile = av[2];
    char *outputfile = av[3];
    int NT = atoi(av[4]);
    char strat = av[5][0];

    if(!powerof2(NT)) {
        printf("Error: NT must be a power of 2\n");
        return 1;
    }

    double start = get_time(), stop;

    /**** PHASE 1: READ INPUT FROM FILE ****/
    int *arr = (int *)calloc(N, sizeof(int));
    if (arr == NULL) {
        perror("Memory allocation failed");
        return 1;
    }

    FILE *input_fp = fopen(inputfile, "rb");  
    if (input_fp == NULL) {
        perror("Failed to open input file");
        free(arr);
        return 1;
    }

    // Reading integers from the input file
    for (int i = 0; i < N; i++) {
        if (fread((arr + i), sizeof(int), 1, input_fp) != 1) {
            printf("Error reading number at index %d\n", i);
            break;
        }
    }
    fclose(input_fp);


    stop = get_time();
    printf("Input time(s): %lf\n", stop - start);
    start = stop;

    /**** PHASE 2: SORT INPUT ****/
    pthread_t threads[NT];
    thread_arg_t * thread_args = (thread_arg_t*) malloc(sizeof(thread_arg_t) * NT);
    int **local_arr = (int**) malloc(sizeof(int *) * NT);
    int *local_arr_size = (int*) malloc(sizeof(int) * NT);

    int t = 0;
    for(t = 0; t < NT; t++){
        thread_args[t].myid = t;
        thread_args[t].N = N;
        thread_args[t].NT = NT;
        thread_args[t].arr = arr;
        thread_args[t].local_arr = local_arr;
        thread_args[t].local_arr_size = local_arr_size;
        pthread_create(&threads[t], NULL, parallel_qs, (void *)&thread_args[t]);
    }

    for (t = 0; t < NT; t++)
        pthread_join(threads[t], NULL);
   
    stop = get_time();
    printf("Sorting time(s): %lf\n", stop - start);
    start = stop;

    /**** PHASE 3: WRITE OUTPUT TO BINARY FILE ****/
    
    FILE *output_fp = fopen(outputfile, "wb");  
    if (output_fp == NULL) {
        perror("Failed to open output file");
        free(arr);
        return 1;
    }

    for (int i = 0; i < N; i++) {
        if (fwrite(&(arr[i]), sizeof(int), 1, output_fp) != 1) {
            perror("Error writing number to output file");
            fclose(output_fp);
            free(arr);
            return 1;
        }
    }

    fclose(output_fp);
    

    for (int i = 0; i < NT; i++) {
        free(local_arr[i]);
    }
    free(local_arr);
    free(local_arr_size);
    free(thread_args);
    free(arr);

    stop = get_time();
    printf("Output time(s): %lf\n", stop - start);
    
    return 0;
}

void* parallel_qs(void* t_args){
    //arguments
    const thread_arg_t* thread_args = (thread_arg_t*) t_args;

    //individual aruments
    int myid = thread_args->myid;

    //shared arguments
    int NT = thread_args->NT;
    int N = thread_args->N;
    int * arr = thread_args->arr;
    int ** local_arr = thread_args->local_arr;
    int * local_arr_size = thread_args->local_arr_size;

    int local_size = N / NT;
    int begin = myid * local_size;
    int end;
    if(myid == NT - 1) {
        end = N - 1;
        local_size = end - begin + 1;
    }
    else {
        end = (myid + 1) * local_size;
    }
    serial_qs(arr, begin, end);

    local_arr_size[myid] = local_size;
    local_arr[myid] = (int *) malloc(sizeof(int) * local_size);
    memcpy(local_arr[myid], &arr[begin], sizeof(int) * local_size);

    //copy all the local arrays back to original array
    int prefix_sum = 0;
    for(int t=0; t<myid; t++){
        prefix_sum += local_arr_size[t];
    }
    memcpy(&arr[prefix_sum], local_arr[myid], local_arr_size[myid]);
    return NULL;
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int powerof2(int num){
    return (num > 0) && ((num & (num - 1)) == 0);
}

// Standard quicksort function
void serial_qs(int *arr, int begin, int end) {
    if (begin >= end) return;

    int pivot = arr[end]; 
    int i = begin - 1;

    for (int j = begin; j < end; j++) {
        if (arr[j] <= pivot) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    int temp = arr[i + 1];
    arr[i + 1] = arr[end];
    arr[end] = temp;

    serial_qs(arr, begin, i);
    serial_qs(arr, i + 2, end);

}
