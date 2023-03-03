#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//////////////////////////// Compile and Run ////////////////////////////
//                                                                     //
//  gcc -Wall -O3 -o sortInt sortInt.c                                 //
//  ./sortInt ./sortInt datSeq32.bin                                   //
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
    char buffer[4];
    int int_buffer[1000];
    size_t elements_read;
    
    // Open binary file for reading
    file = fopen(filename, "rb");
    
    if (file == NULL) {
        printf("Error opening file");
        return 1;
    }
    
    // Read the contents of the file
    elements_read = fread(buffer, sizeof(char), 4, file);
    int n = 32;
    int i = 0;
    while (elements_read == 4 && i < n) {
        // Convert the four bytes into an integer
        int value = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
        int_buffer[i] = value;
        i++;
        
        // Read the next four bytes from the file
        elements_read = fread(buffer, sizeof(char), 4, file);
    }
    
    // Close the file
    fclose(file);
    
    printf("Read %d integers.\n", i);
    
    // Print the integers read from the file
    for (int j = 0; j < i; j++) {
        printf("%d\n", int_buffer[j]);
    }
    
    return 0;
}