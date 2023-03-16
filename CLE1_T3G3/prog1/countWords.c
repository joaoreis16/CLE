#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>


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
    return (strcmp(c, "41") == 0 || strcmp(c, "61") == 0 || strcmp(c, "c3a0") == 0 || strcmp(c, "c3a1") == 0 || strcmp(c, "c3a2") == 0 || strcmp(c, "c3a3") == 0 || strcmp(c, "c380") == 0 || strcmp(c, "c381") == 0 || strcmp(c, "c382") == 0 || strcmp(c, "c383") == 0);
}

bool isVowelE(char* c) {
    return (strcmp(c, "45") == 0 || strcmp(c, "65") == 0 || strcmp(c, "c3a8") == 0 || strcmp(c, "c3a9") == 0 || strcmp(c, "c3aa") == 0 || strcmp(c, "c388") == 0 || strcmp(c, "c389") == 0 || strcmp(c, "c38a") == 0);
}

bool isVowelI(char* c) {
    return (strcmp(c, "49") == 0 || strcmp(c, "69") == 0 || strcmp(c, "c3ac") == 0 || strcmp(c, "c3ad") == 0 || strcmp(c, "c38c") == 0 || strcmp(c, "c38d") == 0);
}

bool isVowelO(char* c) {
    return (strcmp(c, "4f") == 0 || strcmp(c, "6f") == 0 || strcmp(c, "c3b2") == 0 || strcmp(c, "c3b3") == 0 || strcmp(c, "c3b4") == 0 || strcmp(c, "c3b5") == 0 || strcmp(c, "c392") == 0 || strcmp(c, "c393") == 0 || strcmp(c, "c394") == 0 || strcmp(c, "c395") == 0);
}

bool isVowelU(char* c) {
    return (strcmp(c, "55") == 0 || strcmp(c, "75") == 0 || strcmp(c, "c3b9") == 0 || strcmp(c, "c3ba") == 0 || strcmp(c, "c399") == 0 || strcmp(c, "c39a") == 0);
}

bool isVowelY(char* c) {
    return (strcmp(c, "59") == 0 || strcmp(c, "79") == 0);
}

int zanza() {

    char *word_separation[22] = {"20", "09", "0a", "0d", "21", "22", "28", "29", "2e", "2c", "3a", "3b", "3f", "5b", "5d", "2d", "e2809c", "e2809d", "e28093", "e280a6", "c2ab", "c2bb"};

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
        int nWords = 0; int nWordsA = 0; int nWordsE = 0; int nWordsI = 0; int nWordsO = 0; int nWordsU = 0; int nWordsY = 0;
        int total_bytes = 0;
        char actual_char[50];
        char last_char[50];
        bool first_occur[6] = {false, false, false, false, false, false};

        int byte;
        while ((byte = fgetc(file)) != EOF) {
            // printf("%0x\n", byte);
            sprintf(actual_char, "%02x", byte);   //  convert hexadecimal to string

            if (total_bytes == 0) {
                total_bytes = get_char_size(byte);
                strcpy(last_char, "");
            }

            if (total_bytes != 0) {
                strcat(last_char, actual_char);
                total_bytes = total_bytes - 1;
                if (total_bytes != 0) continue;
                strcpy(actual_char, last_char);
            }

            if (contains(actual_char, word_separation, 22)) {
                // printf("separation ");
                inWord = false;
                memset(first_occur, false, sizeof(first_occur));

            } else {
                if (!inWord) {
                    //printf("incrementou ");
                    nWords++;
                    inWord = true;
                }

                if (isVowelA(actual_char)) {
                    if (!first_occur[0]) {
                        nWordsA++;
                        first_occur[0] = true;
                    }
                } else if (isVowelE(actual_char)) {
                    if (!first_occur[1]) {
                        nWordsE++;
                        first_occur[1] = true;
                    }

                } else if (isVowelI(actual_char)) {
                    if (!first_occur[2]) {
                        nWordsI++;
                        first_occur[2] = true;
                    }

                } else if (isVowelO(actual_char)) {
                    if (!first_occur[3]) {
                        nWordsO++;
                        first_occur[3] = true;
                    }

                } else if (isVowelU(actual_char)) {
                    if (!first_occur[4]) {
                        nWordsU++;
                        first_occur[4] = true;
                    }

                } else if (isVowelY(actual_char)) {
                    if (!first_occur[5]) {
                        nWordsY++;
                        first_occur[5] = true;
                    }
                }

            }

            // if (get_char_size(byte) == 1) printf("%s\n", actual_char);
            // total_bytes = get_char_size(byte) - 1;
            
            strcpy(last_char, actual_char); 
        }
        fclose(file); // Close the file

        printf("\n");
        // printing the results
        printf("File name: %s\n", filename);
        printf("Total number of words = %d\n", nWords);
        printf("N. of words with an\n");
        printf("%7s %7s %7s %7s %7s %7s\n", "A", "E", "I", "O", "U", "Y");
        printf("%7d %7d %7d %7d %7d %7d\n\n", nWordsA, nWordsE, nWordsI, nWordsO, nWordsU, nWordsY);
    }
    return 0;
}


void get_valid_chunk() {
    int readBytes = 0;
    int i;
    int word_offset = 0;
    char ch;

    while(readBytes < MAX_CHUNK_SIZE) {
        ch = fgetc(textFiles[currentFile]);

        if (ch == EOF) {
            word_offset = 0;
            currentFile++;
        }

        if (isWordSeparation(ch)) {
        }

        if (isWS(ch)) {
            word_offset = 0;
        }
        
        word_offset += 1;
        chunkBuffer[readBytes++] = ch; // make Buffer global variable
    }  
    if (word_offset != 0) {
        for (i = 0; i < word_offset; i++) {
            chunkBuffer[MAX_CHUNK_SIZE - i] = '\0';
        }
        fseek(textFiles[currentFile], - word_offset, SEEK_CUR);
    }

}