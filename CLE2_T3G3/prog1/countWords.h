/**
 *  \file countWords.h (interface file)
 *
 *  \brief Problem name: Text Processing with Multithreading.
 *
 *  Functions used for text processing.
 *
 *  \author Artur Romão e João Reis - March 2023
 */
#include <stdlib.h>
#include <stdio.h>

#ifndef TEXT_PROC_Funct_H
#define TEXT_PROC_Funct_H

/**
 *  \brief Structure with the filename and file pointer to process.
 *
 *   It also stores the final results of the file processing.
 */
typedef struct {
  int nWords;
  int nWordsA;
  int nWordsE;
  int nWordsI;
  int nWordsO;
  int nWordsU;
  int nWordsY;
  bool is_finished;
} File;

/**
 *  \brief Structure with the chunk data for processing.
 *
 *   It contains the chunk results of the file processing.
 */
typedef struct {
  int index;
  bool is_finished;
  unsigned int *chunk;
  int nWords;
  int nWordsA;
  int nWordsE;
  int nWordsI;
  int nWordsO;
  int nWordsU;
  int nWordsY;
} ChunkData;


/**
 *  \brief Get the size of a char.
 *
 *  \param byte UTF8 encoded character
 *
 *  \return size of the read char.
 */
int get_char_size(int byte);

/**
 *  \brief Check if the char is a separation character.
 *
 *  \param val char read
 *
 *  \return true if the char is a separation char, false if not.
 */
bool is_separation(char *val);

/**
 *  \brief Checks if the given character is the vowel A.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'A', false if not.
 */
bool isVowelA(char* c);

/**
 *  \brief Checks if the given character is the vowel E.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'E', false if not.
 */
bool isVowelE(char* c);

/**
 *  \brief Checks if the given character is the vowel I.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'I', false if not.
 */
bool isVowelI(char* c);

/**
 *  \brief Checks if the given character is the vowel O.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'O', false if not.
 */
bool isVowelO(char* c);

/**
 *  \brief Checks if the given character is the vowel U.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'U', false if not.
 */
bool isVowelU(char* c);

/**
 *  \brief Checks if the given character is the vowel Y.
 *
 *  \param c char read
 *
 *  \return true if the char is an 'Y', false if not.
 */
bool isVowelY(char* c);

/**
 *  \brief Performs text processing of a chunk.
 *
 *  Counts the number of words, and the words containing a specific vowel.

 *  Operation executed by workers.
 *
 *  \param data structure that contains the data needed to process
 *  and will be filled with the results obtained
 *  \param file structure that stores the final results of the file processing
 */
void get_valid_chunk(ChunkData *data, FILE *file);


/**
 *  \brief Performs text processing of a chunk.
 *
 *  Counts the number of words, and the words containing a specific vowel.

 *  Operation executed by workers.
 *
 *  \param data structure that contains the data needed to process
 *  and will be filled with the results obtained
 */
void count_words(ChunkData *data);

#endif /* TEXT_PROC_Funct_H */