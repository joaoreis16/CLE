
#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


/**
 *  \brief Structure with the filename and file pointer to process.
 *
 *   It also stores the final results of the file processing.
 */
struct SubSequence {
  unsigned int *subsequence;
  unsigned int *sorted_subsequence;
  bool is_sorted;
};


/**
 *  \brief Structure with the filename and file pointer to process.
 *
 *   It also stores the final results of the file processing.
 */
struct File {
  char *filename;
  FILE *file;
  unsigned int *sequence;
  unsigned int *sorted_sequence;
  int size;
  struct SubSequence **all_subsequences;
};


/**
 *  \brief Initialization of the data transfer region.
 *
 *  Allocates the memory for an array of structures with the files passed
 *  as argument and initializes it with their names.
 *
 *  \param filename contains the names of the files to be stored
 */
extern void initialize(char *filename);

extern void read_file();

#endif /* MONITOR_H */