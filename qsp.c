#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

typedef struct {
    int myid, N, NT;
    int * arr;
    int ** local_arr; //global view of local arrays
    int * local_arr_size;
    int * medians, * pivots, * splitpoints;
    pthread_barrier_t * pair_barriers, * group_barriers;
    
} thread_arg_t;

double get_time();
int powerof2(int);
void* parallel_qs(void*);
void local_sort(int *, int, int);
void global_sort(thread_arg_t*, int, int);


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
    int medians[NT], pivots[NT], splitpoints[NT];

    int t = 0;
    const int pair_barrier_size = NT; 
    const int group_barrier_size = NT - 1;
    pthread_barrier_t pair_barriers[pair_barrier_size];
    pthread_barrier_t group_barriers[group_barrier_size];

    for (int i = 0; i < pair_barrier_size; i++)
        pthread_barrier_init(&pair_barriers[i], NULL, 2); 
    int gb_count = 0;
    for (int i = 1; i < NT; i*=2){
        int nt = NT / i;
        for(int j = 0; j < i; j++){
            pthread_barrier_init(&group_barriers[gb_count], NULL, nt);
            gb_count++;
        }
    }

    for(t = 0; t < NT; t++){
        thread_args[t].myid = t;
        thread_args[t].N = N;
        thread_args[t].NT = NT;
        thread_args[t].arr = arr;
        thread_args[t].local_arr = local_arr;
        thread_args[t].local_arr_size = local_arr_size;
        thread_args[t].medians = medians;
        thread_args[t].pivots = pivots;
        thread_args[t].splitpoints = splitpoints;
        thread_args[t].pair_barriers = pair_barriers;
        thread_args[t].group_barriers = group_barriers;
        pthread_create(&threads[t], NULL, parallel_qs, (void *)&thread_args[t]);
    }

    for (t = 0; t < NT; t++)
        pthread_join(threads[t], NULL);
   
    stop = get_time();
    printf("Sorting time(s): %lf\n", stop - start);
    start = stop;

    for (int i = 0; i < pair_barrier_size; i++)
        pthread_barrier_destroy(&pair_barriers[i]);
    for (int i = 0; i < group_barrier_size; i++)
        pthread_barrier_destroy(&group_barriers[i]);


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
    thread_arg_t* thread_args = (thread_arg_t*) t_args;
    pthread_barrier_t* group_barriers = thread_args->group_barriers;

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
        end = (myid + 1) * local_size - 1;
    }
    local_sort(arr, begin, end);

    local_arr_size[myid] = local_size;
    local_arr[myid] = (int *) malloc(sizeof(int) * local_size);
    memcpy(local_arr[myid], &arr[begin], sizeof(int) * local_size);

    global_sort(thread_args, NT, 1);
    //use a barrier to wait for all the threads
    pthread_barrier_wait(&group_barriers[0]);

    //copy all the local arrays back to original array
    int prefix_sum = 0;
    for(int t=0; t<myid; t++){
        prefix_sum += local_arr_size[t];
    }
    memcpy(&arr[prefix_sum], local_arr[myid], local_arr_size[myid]);
    return NULL;
}

void global_sort(thread_arg_t *thread_args, int size, int gbt){
    int ** local_arr = thread_args->local_arr;
    int * local_arr_size = thread_args->local_arr_size;
    int myid = thread_args->myid;
    int * medians = thread_args->medians;
    int * pivots = thread_args->pivots;
    int * splitpoints = thread_args->splitpoints;
    pthread_barrier_t * pair_barriers = thread_args->pair_barriers;
    pthread_barrier_t * group_barriers = thread_args->group_barriers;

    if(size == 1) return;
    int localid = myid % size;
    int groupid = myid / size;

    int* local_array = local_arr[myid];
    int len = local_arr_size[myid];
    int median, pivot;
    median = local_arr[myid][len/2];
    medians[myid] = median;
    pthread_barrier_wait(&group_barriers[(gbt - 1) + groupid]);
    // if locid==0 pivot[group]=select the median of the local_arr and save that in pivots array for everything  from myid upto size number of indices       
    if (localid == 0){
        pivot = medians[myid];
        for(int i=0; i<size; i++){
            pivots[myid + i] = pivot;
        }
    }
    pthread_barrier_wait(&group_barriers[(gbt - 1) + groupid]);
    
    pivot = pivots[myid];
    int split = 0, i, j, k;
    for(i = 0; i < len && local_array[i] < pivot; i++);
    split = i;
    splitpoints[myid] = split;

    pthread_barrier_wait(&group_barriers[(gbt - 1) + groupid]);

    int merged_array_size;
    int* merged_array;
    int other_half;
    int barrier_index;
    if(localid<size/2){
        other_half = myid + size/2;
        merged_array_size = split + splitpoints[other_half];
        barrier_index = myid; //NT pair barriers created and upper half waits on a lower half barrier (for ease of implementation)
    }
    else{
        other_half = myid - size/2;
        merged_array_size = (len - split ) + (local_arr_size[other_half] - splitpoints[other_half]);
        barrier_index = other_half;
    }
    merged_array = (int * ) malloc(sizeof(int) * merged_array_size);

    //merging process - if lower half, merge lowerparts, else merge upperparts
    i = 0, j = 0, k = 0;
    if(localid < size/2) {
        // Merging the lower half
        while (i < split && j < splitpoints[other_half]) {
            if (local_array[i] < local_arr[other_half][j]) {
                merged_array[k++] = local_array[i++];
            } else {
                merged_array[k++] = local_arr[other_half][j++];
            }
        }

        // Copy remaining elements
        while (i < split) {
            merged_array[k++] = local_array[i++];
        }
        while (j < splitpoints[other_half]) {
            merged_array[k++] = local_arr[other_half][j++];
        }
    } else {
        int split_other = splitpoints[other_half];
    
        // Merging the upper half
        while (i < len - split && j < local_arr_size[other_half] - split_other) {
            if (local_array[i + split] < local_arr[other_half][j + split_other]) {
                merged_array[k++] = local_array[i + split];
                i++;
            } else {
                merged_array[k++] = local_arr[other_half][j + split_other];
                j++;
            }
        }
    
        // Copy remaining elements
        while (i < len - split) {
            merged_array[k++] = local_array[i + split];
            i++;
        }
        while (j < local_arr_size[other_half] - split_other) {
            merged_array[k++] = local_arr[other_half][j + split_other];
            j++;
        }
    }

    //wait array should go inside this
    pthread_barrier_wait(&pair_barriers[barrier_index]);
    
    free(local_arr[myid]);
    local_arr[myid] = merged_array;
    local_arr_size[myid] = merged_array_size;

    


    global_sort(thread_args, size/2, gbt * 2);
}


// Standard quicksort function
void local_sort(int *arr, int begin, int end) {
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

    local_sort(arr, begin, i);
    local_sort(arr, i + 2, end);

}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int powerof2(int num){
    return (num > 0) && ((num & (num - 1)) == 0);
}