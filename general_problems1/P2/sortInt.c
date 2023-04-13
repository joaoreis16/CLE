#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//////////////////////////// Compile and Run ////////////////////////////
//                                                                     //
//  gcc -Wall -O3 -o sortInt sortInt.c                                 //
//  ./sortInt datSeq32.bin                                             //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

void print(int *val, int N) {
    for (int j = 0; j < N; j++) {
        printf("%d\n", val[j]);
    } 
}

void validate(int *val, int N) {
    int i;
    for (i = 0; i < N; i++) {

        if (i == (N - 1))  {
            printf ("Everything is OK!\n");
            break;
        }

        if (val[i] > val[i+1]) { 
            printf ("Error in position %d between element %d and %d\n", i, val[i], val[i+1]);
            break;
        }

    }
}

void compareAndSwap(int *val, int i, int j, int dir) {
    if ((val[i] > val[j]) == dir) {
        int temp = val[i];
        val[i] = val[j];
        val[j] = temp;
    }
}

void bitonicMerge(int *val, int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; i++) {
            compareAndSwap(val, i, i + k, dir);
        }
        bitonicMerge(val, low, k, dir);
        bitonicMerge(val, low + k, k, dir);
    }
}

void bitonicSortRecursive(int *val, int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonicSortRecursive(val, low, k, !dir);
        bitonicSortRecursive(val, low + k, k, dir);
        bitonicMerge(val, low, cnt, dir);
    }
}

void bitonicSort(int *val, int N) {
    bitonicSortRecursive(val, 0, N, 1);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("[usage]: %s filename\n", argv[0]);
        return 1;
    }

    if (chdir("dataset") == -1) {
        perror("chdir");
        return 1;
    }

    FILE *file;
    char *filename = argv[1];
    
    // Open binary file for reading
    file = fopen(filename, "rb");
    
    if (file == NULL) {
        printf("Error opening the file");
        return 1;
    }

    // Read the header of the binary file
    int N_values = 0;
    if (fread(&N_values, sizeof(int), 1, file) != 1) {
        printf("Error reading the file");
        return 1;
    }
    printf("number of values = %d\n", N_values);

    int *sequence = (int*)malloc(N_values * sizeof(int));
    int i = 0;

    // Read the contents of the file
    int num;
    while(fread(&num, sizeof(int), 1, file) == 1) {
        sequence[i] = num;
        i++;
    }
    
    // Close the file
    fclose(file);

    quicksort(sequence, 0, N_values - 1);
    
    // Print the integers read from the file
    // printf("sorted sequence:\n");
    // print(sequence, N_values);

    validate(sequence, N_values);
    return 0;
}