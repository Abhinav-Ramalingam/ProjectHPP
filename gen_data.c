#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void generate_uniform_numbers(int N, const char *filename) {
    srand(time(NULL));

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        int num = rand();  // Uniform distribution from 0 to RAND_MAX
        fwrite(&num, sizeof(int), 1, file);
    }

    fclose(file);
}

void generate_exponential_numbers(int N, const char *filename) {
    srand(time(NULL));

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        double num = -log((double)rand() / RAND_MAX);  // Exponential distribution with lambda = 1
        fwrite(&num, sizeof(double), 1, file);
    }

    fclose(file);
}

void generate_normal_numbers(int N, const char *filename) {
    srand(time(NULL));

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        double num = 0.0;
        for (int j = 0; j < 12; j++) {
            num += (rand() / (double)RAND_MAX);  // Central limit theorem for normal distribution
        }
        num -= 6.0;  // Normalize to mean = 0
        fwrite(&num, sizeof(double), 1, file);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <N> <dist> <filename>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    char dist = argv[2][0];
    const char *filename = argv[3];

    switch (dist) {
        case 'r':
            generate_uniform_numbers(N, filename);
            break;
        case 'e':
            generate_exponential_numbers(N, filename);
            break;
        case 'n':
            generate_normal_numbers(N, filename);
            break;
        default:
            fprintf(stderr, "Illegal argument: dist must be 'r' (uniform), 'e' (exponential), or 'n' (normal)\n");
            return 1;
    }

    return 0;
}
