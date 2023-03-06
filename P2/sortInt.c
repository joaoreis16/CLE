#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//////////////////////////// Compile and Run ////////////////////////////
//                                                                     //
//  gcc -Wall -O3 -o sortInt sortInt.c                                 //
//  ./sortInt datSeq32.bin                                             //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

void validate(int *val, int N) {
    int i;
    for (i = 0; i < N - 1; i++) {
        if (val[i] > val[i+1]) { 
            printf ("Error in position %d between element %d and %d\n", i, val[i], val[i+1]);
            break;
        }

        if (i == (N - 1)) printf ("Everything is OK!\n");
    }
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

    int sequence[N_values];
    int i = 0;

    // Read the contents of the file
    int num;
    while(fread(&num, sizeof(int), 1, file) == 1) {
        sequence[i] = num;
        i++;
    }
    
    // Close the file
    fclose(file);
    
    // Print the integers read from the file
    for (int j = 0; j < N_values; j++) {
        printf("%d\n", sequence[j]);
    }

    validate(sequence, N_values);
    return 0;
}