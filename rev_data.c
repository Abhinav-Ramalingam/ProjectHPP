#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <N> <inputfile> <outputfile>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    char *inputfile = argv[2];
    char *outputfile = argv[3];

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

    // Phase 2: Reverse the array
    for (int i = 0; i < N / 2; i++) {
        int temp = arr[i];
        arr[i] = arr[N - i - 1];
        arr[N - i - 1] = temp;
    }

    // Phase 3: Write the reversed array to output binary file
    FILE *output_fp = fopen(outputfile, "wb");
    if (output_fp == NULL) {
        perror("Failed to open output file");
        free(arr);
        return 1;
    }

    size_t written_elements = fwrite(arr, sizeof(int), N, output_fp);
    if (written_elements != N) {
        printf("Error writing data to binary file\n");
        fclose(output_fp);
        free(arr);
        return 1;
    }
    fclose(output_fp);

    free(arr);
    printf("Reversed numbers have been written to %s\n", outputfile);
    return 0;
}
