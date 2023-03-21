#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "shared.h"

/** \brief max number of bytes per chunk */
extern int maxBytesPerChunk;

char *word_separation[24] = {"00", "20", "09", "0a", "0d", "21", "22", "28", "29", "2e", "2c", "3a", "3b", "3f", "5b", "5d", "2d", "e2809c", "e2809d", "e28093", "e280a6", "c2ab", "c2bb", "e28094"};

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

void count_words(struct ChunkData *data) {

    bool inWord = false;
    int nWords = 0; int nWordsA = 0; int nWordsE = 0; int nWordsI = 0; int nWordsO = 0; int nWordsU = 0; int nWordsY = 0;
    int total_bytes = 0;
    char actual_char[50];
    char last_char[50];
    bool first_occur[6] = {false, false, false, false, false, false};

    for(int k = 0; k < maxBytesPerChunk; k++) {
        int byte = data->chunk[k];
        
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

        if (contains(actual_char, word_separation, 24)) {
            inWord = false;
            memset(first_occur, false, sizeof(first_occur));

        } else {
            if (!inWord) {
                if (strcmp(actual_char, "27") != 0 && strcmp(actual_char, "e28098") != 0 && strcmp(actual_char, "e28099") != 0) {
                    nWords++;
                    inWord = true;
                }
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
        strcpy(last_char, actual_char); 
    }

    data->nWords  = nWords;
    data->nWordsA = nWordsA;
    data->nWordsE = nWordsE;
    data->nWordsI = nWordsI;
    data->nWordsO = nWordsO;
    data->nWordsU = nWordsU;
    data->nWordsY = nWordsY;

}


void get_valid_chunk(struct ChunkData *data, struct File *file) {
    int bytes_readed = 0;
    int word_offset  = 0;
    unsigned char actual_char[50];
    char last_char[50];
    int total_bytes = 0;

    while (bytes_readed < maxBytesPerChunk) {
        int byte = fgetc(file->file);
        sprintf(actual_char, "%02x", byte);   //  convert hexadecimal to string

        if (byte == EOF) {
            word_offset = 0;
            // printf("fim da ficheiro\n");
            data->is_finished = true;
            break;
        }

        if (total_bytes == 0) {
            total_bytes = get_char_size(byte);
            strcpy(last_char, "");
        }

        // if char is multi-byte
        if (total_bytes != 0) {
            strcat(last_char, actual_char);
            
            total_bytes--;
            word_offset++;

            if (total_bytes != 0) {
                data->chunk[bytes_readed++] = byte;   // Fill chunk with content
                continue;
            }
            strcpy(actual_char, last_char);
        } 
        
        if (contains(actual_char, word_separation, 24)) {
            word_offset = 0;
        }

        data->chunk[bytes_readed++] = byte;   // Fill chunk with content
    }

    if (word_offset != 0) {
        for (int i = 1; i <= word_offset; i++) {
            data->chunk[maxBytesPerChunk - i] = '\0';
        }
        fseek(file->file, - word_offset, SEEK_CUR);
    }

    printf("\n>> print da chunk\n");
    for(int loop = 0; loop < maxBytesPerChunk; loop++)
        printf("%c", (char)data->chunk[loop]);

}