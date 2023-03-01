#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h> 

#define SIZE_WORD_SEPARATION 20

//////////////// Compile and Run ///////////////
//                                            //
//  gcc -Wall -O3 -o countWords countWords.c  //
//  ./countWords text0.txt                    //
//                                            //
//////////////////////////////////////////////// 

typedef enum {false, true} bool;

bool contains(char val, char arr[], size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (arr[i] == val) return true;
    }
    return false;
}

bool isApostrophe(char * character) {
    if (strcmp(character, "0xe28098") == 0|| strcmp(character, "0xe28099") == 0) return true;
    return false;
}

void print_array(char arr[]) {
    for (size_t i = 0; i < SIZE_WORD_SEPARATION; i++) {
        printf("%02x, ", arr[i]);
    }
    printf("\n");
}

// main function
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "pt_PT.UTF-8");

    if (argc < 2) {
        printf("[usage]: %s file1 [file2 ...]\n", argv[0]);
        return 1;
    }

    if(chdir("dataset") == -1) {
        perror("chdir");
        return 1;
    }

    char word_separation[SIZE_WORD_SEPARATION] = {0x20, 0x9, 0xa, 0xd, 0x21, 0x22, 0x28, 0x29, 0x2e, 0x2c, 0x3a, 0x3b, 0x3f, 0x5b, 0x5d, 0x2d, 0xe2809c, 0xe2809d, 0xe28093, 0xe280a6};
    print_array(word_separation);

    int i;
    for (i = 1; i < argc; i++) {
        char *filename = argv[i];

        FILE *file = fopen(filename, "rb");
        if (file == NULL) {
            perror("[error] file not found!");
            return 1;
        }

        char content[256];
        int nWords = 0; int nWordswA = 0; int nWordswE = 0; int nWordswI = 0; int nWordswO = 0; int nWordswU = 0; int nWordsY  = 0;
        bool inWord = false;
        bool new_word = false;
        bool three_hex = false;
        bool two_hex = false;

        char _cmd[100] = "od -A x -t x1 %s";
        char all_cmd[150];
        sprintf(all_cmd, _cmd, filename);

        // read file and get the content of the file in hexadecimal
        FILE *cmd = popen(all_cmd, "r");
        while (fgets(content, sizeof(content), cmd) != 0) {
            content[strlen(content) - 1] = '\0';    // remove \n 

            char * hex;
            hex = strtok (content," ,.-");

            // splitting content by space
            while (hex != NULL) {

                // ignore the begining of the line, for example: 000000
                if (strlen(hex) < 5) {     

                    if (strcmp(hex, "e2") == 0) {
                        three_hex = true;
                        hex = strtok (NULL, " ,.-");
                        continue;
                    }

                    if (strcmp(hex, "c3") == 0) {
                        two_hex = true;
                        hex = strtok (NULL, " ,.-");
                        continue;
                    }

                    // convert hexadecimal char * to hexadecimal char
                    char hexadecimal;   

                    if (three_hex) {
                        if (strcmp(hex, "80") == 0) {
                            hex = strtok (NULL, " ,.-");
                            continue;
                        }

                        char _hex[100] = "e280%s";
                        char new_hex[32];
                        sprintf(new_hex, _hex, hex);
                        hex = new_hex;

                        hexadecimal = (char) strtol(new_hex, NULL, 16);
                        three_hex = false;

                    } else if (two_hex) {
                        char _hex[100] = "23%s";
                        char new_hex[32];
                        sprintf(new_hex, _hex, hex);
                        hex = new_hex;

                        hexadecimal = (char) strtol(new_hex, NULL, 16);
                        two_hex = false;
                        
                    } else {
                        hexadecimal = (char) strtol(hex, NULL, 16);
                    }

                    if (contains(hexadecimal, word_separation, SIZE_WORD_SEPARATION)) {
                        printf("[separation] ");
                        inWord = false;
                        new_word = false;

                    } else if (!new_word && !inWord) {
                        new_word = true;
                        inWord = true;

                    } else {
                        new_word = false;
                        inWord = true;
                    }

                    if (inWord && new_word && !isApostrophe(hex)) {
                        nWords = nWords + 1;
                        printf("[INCREMENTOU] ");
                    }

                    printf("%c %02x (%s)\n", hexadecimal, hexadecimal, hex);
                }

                hex = strtok (NULL, " ,.-");
            }
        }
        pclose(cmd);        
        fclose(file);

        // printing the results
        printf("File name: %s\n", filename);
        printf("Total number of words = %d\n", nWords);
        printf("N. of words with an\n");
        printf("%7s %7s %7s %7s %7s %7s\n", "A", "E", "I", "O", "U", "Y");
        printf("%7d %7d %7d %7d %7d %7d\n\n", nWordswA, nWordswE, nWordswI, nWordswO, nWordswU, nWordsY);
    }
    return 0;
}

// char     hex pc      hex prof
//  –       ffffff93    (e28093)
//  Ó       ffffff93    (2393)
