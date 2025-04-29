#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_random_numbers(int N, const char *filename) {
    srand(time(NULL));

    FILE *file = fopen(filename, "wb");  
    if (file == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        int num = rand();
        fwrite(&num, sizeof(int), 1, file);   
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    generate_random_numbers(N, "numbers");  

    return 0;
}
