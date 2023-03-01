#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//////////////// Compile and Run ///////////////
//                                            //
//  gcc -Wall -O3 -o countWords version3.c    //
//  ./countWords text.txt                     //
//                                            //
//////////////////////////////////////////////// 

typedef enum {false, true} bool;

int get_char_size(int byte) {
    if (byte < 192) {
        return 1;   // 0x0XXXXXXX

    } else if(byte >= 192 && byte < 224) {
        return 2;   // 0x110XXXXX

    } else if(byte >= 224 && byte < 240) {
        return 3;   // 0x1110XXXX

    } else {
        return 4;   // 0x11110XXX
    }
}

bool contains(char *val, char *arr[], size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (strcmp(arr[i], val) == 0) return true;
    } 
    return false;
}

bool isVowelA(char* c) {
    return (strcmp(c, "61") || strcmp(c, "c3a0") || strcmp(c, "c3a1") || strcmp(c, "c3a2") || strcmp(c, "c3a3"));
}

bool isVowelE(char* c) {
    return (strcmp(c, "65") || strcmp(c, "c3a8") || strcmp(c, "c3a9") || strcmp(c, "c3aa"));
}

bool isVowelI(char* c) {
    return (strcmp(c, "69") || strcmp(c, "c3ac") || strcmp(c, "c3ad"));
}

bool isVowelO(char* c) {
    return (strcmp(c, "6f") || strcmp(c, "c3b2") || strcmp(c, "c3b3") || strcmp(c, "c3b4") || strcmp(c, "c3b5"));
}

bool isVowelU(char* c) {
    return (strcmp(c, "75") || strcmp(c, "c3b9") || strcmp(c, "c3ba"));
}

bool isVowelY(char* c) {
    return (strcmp(c, "79"));
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("[usage]: %s file1 [file2 ...]\n", argv[0]);
        return 1;
    }

    if (chdir("dataset") == -1) {
        perror("chdir");
        return 1;
    }

    char *word_separation[20] = {"20", "9", "0a", "d", "21", "22", "28", "29", "2e", "2c", "3a", "3b", "3f", "5b", "5d", "2d", "e2809c", "e2809d", "e28093", "e280a6"};

    FILE *file;
    int i;
    for (i = 1; i < argc; i++) {
        char *filename = argv[i];
        file = fopen(filename, "rb"); // Open the file in binary read mode

        if (file == NULL) { // Check if the file exists or not
            printf("Error: File does not exist\n");
            return 0;
        }

        bool inWord = false;
        int nWords = 0; int nWordswA = 0; int nWordswE = 0; int nWordswI = 0; int nWordswO = 0; int nWordswU = 0; int nWordsY  = 0;
        int total_bytes = 0;
        char actual_char[50];
        char last_char[50];
        bool first_occur[6] = {false, false, false, false, false, false};

        int byte;
        while ((byte = fgetc(file)) != EOF) {
            // printf("%0x\n", byte);
            sprintf(actual_char, "%02x", byte);   //  convert hexadecimal to string

            // e2 80 a7
            
            if (total_bytes != 0) {
                strcat(last_char, actual_char);
                // printf("concatenate : %s\n", last_char);
                total_bytes = total_bytes - 1;
                if (total_bytes != 0) continue;
                strcpy(actual_char, last_char);
            }

            if (contains(actual_char, word_separation, 20)) {
                printf("separation ");
                inWord = false;

            } else {
                if (!inWord) {
                    nWords = nWords + 1;
                    printf("incrementou ");
                    inWord = true;
                }

            }

            if (get_char_size(byte) == 1) printf("%s\n", actual_char);

            total_bytes = get_char_size(byte) - 1;
            strcpy(last_char, actual_char); 
        }
        fclose(file); // Close the file

        printf("\n");
        // printing the results
        printf("File name: %s\n", filename);
        printf("Total number of words = %d\n", nWords);
        printf("N. of words with an\n");
        printf("%7s %7s %7s %7s %7s %7s\n", "A", "E", "I", "O", "U", "Y");
        printf("%7d %7d %7d %7d %7d %7d\n\n", nWordswA, nWordswE, nWordswI, nWordswO, nWordswU, nWordsY);
    }
    return 0;
}