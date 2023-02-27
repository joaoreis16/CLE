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

print(char *array, int length){
    printf("word_separation = [");
    for (int i = 0; i < length; i++) { 
        printf("%02x,", array[i]);
    }
    printf("]\n");
}


bool contains(int val, char arr[], size_t n) {
    for (size_t i = 0; i < n; i++) {
        /* printf("lista: %02x\n", arr[i]);
        printf("hex: %02x\n", val); */
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
    char word_separation[16] = {0x20, 0x9, 0xa, 0xd, 0x21, 0x22, 0x28, 0x29, 0x2e, 0x2c, 0x3a, 0x3b, 0x3f, 0x5b, 0x5d, 0xe2};
    print(word_separation, 16);

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

        char _cmd[100] = "od -A x -t x1 %s";
        char all_cmd[150];
        sprintf(all_cmd, _cmd, filename);

        // read file and get the content of the file in hexadecimal
        FILE *cmd = popen(all_cmd, "r");
        while (fgets(content, sizeof(content), cmd) != 0) {
            char hex[100];
            hex = strtok (content," ,.-");

            // splitting content by space
            while (hex != NULL) {
                
                // convert hexadecimal char * to 
                int * hexadecimal = (int *)(hex);
                /* printf("%ls", hexadecimal); */

                if (contains((int) * hexadecimal, word_separation, 16)) {
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

