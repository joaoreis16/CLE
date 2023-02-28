#include <stdio.h>

//////////////// Compile and Run ///////////////
//                                            //
//  gcc -Wall -O3 -o countWords version3.c    //
//  ./countWords                              //
//                                            //
//////////////////////////////////////////////// 

int main() {
    FILE *fp;
    char buffer[1];
    size_t readSize;

    fp = fopen("dataset/text.txt", "rb"); // Open the file in binary read mode

    if (fp == NULL) { // Check if the file exists or not
        printf("Error: File does not exist\n");
        return 0;
    }

    while ((readSize = fread(buffer, 1, 1, fp)) > 0) { // Read one byte at a time until end of file
        printf("%c", buffer[0]); // Print the byte
    }

    fclose(fp); // Close the file
    return 0;
}