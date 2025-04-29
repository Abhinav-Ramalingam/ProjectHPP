#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <N> <filename>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    char *inputfile = argv[2];

    int *arr = (int *)calloc(N, sizeof(int));
    if (!arr) return 1;

    FILE *fp = fopen(inputfile, "rb");
    if (!fp) {
        free(arr);
        return 1;
    }

    fread(arr, sizeof(int), N, fp);
    fclose(fp);

    for (int i = 0; i < N; i++) printf("%d ", arr[i]);
    printf("\n");
    free(arr);
    return 0;
}
