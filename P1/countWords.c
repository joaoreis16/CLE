#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h> 

//////////////// Compile and Run ///////////////
//                                            //
//  gcc -Wall -O3 -o countWords countWords.c  //
//  ./countWords text0.txt                    //
//                                            //
//////////////////////////////////////////////// 

typedef enum {false, true} bool;

bool contains(int * val, int *arr, size_t n) {
    for(size_t i = 0; i < n; i++) {
        if (arr[i] == val) return true;
    }
    return false;
}

// main function
int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("[usage]: %s filenames\n", argv[0]);
        return 1;
    }

    chdir("dataset");
    int word_separation[16] = {0x20, 0x9, 0xa, 0xd, 0x21, 0x22, 0x28, 0x29, 0x2e, 0x2c, 0x3a, 0x3b, 0x3f, 0x5b, 0x5d, 0xe2};

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

        char *_cmd = "od -A x -t x1 %s";
        char all_cmd[150];
        sprintf(all_cmd, _cmd, filename);

        // read file and get the content of the file in hexadecimal
        FILE *cmd = popen(all_cmd, "r");
        while (fgets(content, sizeof(content), cmd) != 0) {
            char * hex;
            hex = strtok (content," ,.-");

            // splitting content by space
            while (hex != NULL) {
                
                // convert hexadecimal char to int
                int * hexadecimal = (int *)(hex);

                if (contains(hexadecimal, word_separation, 16)) {
                    printf("separation char: ");
                    inWord = false;
                    new_word = false;

                } else if (!new_word && !inWord) {
                    new_word = true;
                    inWord = true;

                } else {
                    new_word = false;
                    inWord = true;
                }

                if (inWord && new_word) {
                    nWords = nWords + 1;
                }

                printf ("%s\n", hex);
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

