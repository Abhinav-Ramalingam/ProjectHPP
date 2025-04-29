#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <N> <filename>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    char *inputfile = argv[2];

    // Phase 1: Read input from binary file
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

    size_t read_elements = fread(arr, sizeof(int), N, input_fp);
    if (read_elements != N) {
        printf("Error reading data from binary file\n");
        fclose(input_fp);
        free(arr);
        return 1;
    }
    fclose(input_fp);

    // Phase 2: Check if the numbers are in ascending order
    int deviations = 0;
    for (int i = 1; i < N; i++) {
        if (arr[i] < arr[i - 1]) {
            deviations++;
            printf("Deviation at index %d: %d -> %d\n", i - 1, arr[i - 1], arr[i]);
        }
    }

    printf("Total deviations: %d\n", deviations);

    free(arr);
    return 0;
}
