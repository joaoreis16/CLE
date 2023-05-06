#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "countWords.h"

/** \brief max number of bytes per chunk */
extern int maxBytesPerChunk;


/**
 *  \brief Get the size of a char.
 *
 *  \param byte UTF8 encoded character
 *
 *  \return size of the read char.
 */
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


/**
 *  \brief Check if the char is a separation character.
 *
 *  \param val char read
 *
 *  \return true if the char is a separation char, false if not.
 */
bool is_separation(char* val) {
    char *word_separation[24] = {"00", "20", "09", "0a", "0d", "21", "22", "28", "29", "2e", "2c", "3a", "3b", "3f", "5b", "5d", "2d", "e2809c", "e2809d", "e28093", "e280a6", "c2ab", "c2bb", "e28094"};
    for (size_t i = 0; i < 24; i++) {
        if (strcmp(word_separation[i], val) == 0) return true;
    } 
    return false;
}


/**
 *  \brief Checks if the given character is the vowel A.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'A', false if not.
 */
bool isVowelA(char* c) {
    return (strcmp(c, "41") == 0 || strcmp(c, "61") == 0 || strcmp(c, "c3a0") == 0 || strcmp(c, "c3a1") == 0 || strcmp(c, "c3a2") == 0 || strcmp(c, "c3a3") == 0 || strcmp(c, "c380") == 0 || strcmp(c, "c381") == 0 || strcmp(c, "c382") == 0 || strcmp(c, "c383") == 0);
}


/**
 *  \brief Checks if the given character is the vowel E.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'E', false if not.
 */
bool isVowelE(char* c) {
    return (strcmp(c, "45") == 0 || strcmp(c, "65") == 0 || strcmp(c, "c3a8") == 0 || strcmp(c, "c3a9") == 0 || strcmp(c, "c3aa") == 0 || strcmp(c, "c388") == 0 || strcmp(c, "c389") == 0 || strcmp(c, "c38a") == 0);
}


/**
 *  \brief Checks if the given character is the vowel I.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'I', false if not.
 */
bool isVowelI(char* c) {
    return (strcmp(c, "49") == 0 || strcmp(c, "69") == 0 || strcmp(c, "c3ac") == 0 || strcmp(c, "c3ad") == 0 || strcmp(c, "c38c") == 0 || strcmp(c, "c38d") == 0);
}


/**
 *  \brief Checks if the given character is the vowel O.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'O', false if not.
 */
bool isVowelO(char* c) {
    return (strcmp(c, "4f") == 0 || strcmp(c, "6f") == 0 || strcmp(c, "c3b2") == 0 || strcmp(c, "c3b3") == 0 || strcmp(c, "c3b4") == 0 || strcmp(c, "c3b5") == 0 || strcmp(c, "c392") == 0 || strcmp(c, "c393") == 0 || strcmp(c, "c394") == 0 || strcmp(c, "c395") == 0);
}


/**
 *  \brief Checks if the given character is the vowel U.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'U', false if not.
 */
bool isVowelU(char* c) {
    return (strcmp(c, "55") == 0 || strcmp(c, "75") == 0 || strcmp(c, "c3b9") == 0 || strcmp(c, "c3ba") == 0 || strcmp(c, "c399") == 0 || strcmp(c, "c39a") == 0);
}


/**
 *  \brief Checks if the given character is the vowel Y.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'Y', false if not.
 */
bool isVowelY(char* c) {
    return (strcmp(c, "59") == 0 || strcmp(c, "79") == 0);
}


/**
 *  \brief Performs text processing of a chunk.
 *
 *  Counts the number of words, and the words containing a specific vowel.

 *  Operation executed by workers.
 *
 *  \param data structure that contains the data needed to process
 *  and will be filled with the results obtained
 */
void count_words(struct ChunkData *data) {
    bool inWord = false;
    data->nWords = 0; data->nWordsA = 0; data->nWordsE = 0; data->nWordsI = 0; data->nWordsO = 0; data->nWordsU = 0; data->nWordsY = 0;
    int total_bytes = 0;
    char actual_char[50];
    char last_char[50];
    bool first_occur[6] = {false, false, false, false, false, false};

    for(int k = 0; k < data->chunk_size; k++) {
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

        if (is_separation(actual_char)) {
            inWord = false;
            memset(first_occur, false, sizeof(first_occur));

        } else {
            if (!inWord) {
                if (strcmp(actual_char, "27") != 0 && strcmp(actual_char, "e28098") != 0 && strcmp(actual_char, "e28099") != 0) {
                    data->nWords++;
                    inWord = true;
                }
            }

            if (isVowelA(actual_char)) {
                if (!first_occur[0]) {
                    data->nWordsA++;
                    first_occur[0] = true;
                }
            } else if (isVowelE(actual_char)) {
                if (!first_occur[1]) {
                    data->nWordsE++;
                    first_occur[1] = true;
                }

            } else if (isVowelI(actual_char)) {
                if (!first_occur[2]) {
                    data->nWordsI++;
                    first_occur[2] = true;
                }

            } else if (isVowelO(actual_char)) {
                if (!first_occur[3]) {
                    data->nWordsO++;
                    first_occur[3] = true;
                }

            } else if (isVowelU(actual_char)) {
                if (!first_occur[4]) {
                    data->nWordsU++;
                    first_occur[4] = true;
                }

            } else if (isVowelY(actual_char)) {
                if (!first_occur[5]) {
                    data->nWordsY++;
                    first_occur[5] = true;
                }
            }

        }
        strcpy(last_char, actual_char); 
    }
}

/**
 *  \brief Checks whether a chunk is valid or not.
 *
 *  Reads the whole chunk and checks if it ends in a middle of a word or in the middle
 *  of a multi-byte character. 
 *  Seeks the last valid position in the text so that the next worker can start from there.
 *  Operation executed by workers.
 *
 *  \param data structure that contains the data needed to process
 *  and will be filled with the results obtained
 *  \param file structure that stores the final results of the file processing
 */
void get_valid_chunk(struct ChunkData *data, FILE *file) {
    int bytes_read = 0;
    int word_offset  = 0;
    char actual_char[50];
    char last_char[50];
    int total_bytes = 0;
    int num_of_bytes = 0;
    while (bytes_read < maxBytesPerChunk) {
        int byte = fgetc(file);
        num_of_bytes++;

        if (byte == EOF) {
            word_offset = 0;
            data->is_finished = true;
            break;
        }

        if (total_bytes == 0) {
            total_bytes = get_char_size(byte);
            strcpy(last_char, "");
        }

        sprintf(actual_char, "%02x", byte);   //  convert hexadecimal to string

        // if char is multi-byte
        if (total_bytes != 0) {
            strcat(last_char, actual_char);
            
            total_bytes--;
            word_offset++;

            if (total_bytes != 0) {
                data->chunk[bytes_read++] = byte;   // Fill chunk with content
                continue;
            }
            strcpy(actual_char, last_char);   
        }
        
        if (is_separation(actual_char)) word_offset = 0;

        data->chunk[bytes_read++] = byte;   // Fill chunk with content
    }

    for (int i = 1; i <= word_offset; i++) {
        data->chunk[maxBytesPerChunk - i] = '\0';
        num_of_bytes--;
    }

    data->chunk_size = num_of_bytes - 1;
    fseek(file, - word_offset, SEEK_CUR);
}