
#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


/**
 *  \brief Structure with the filename and file pointer to process.
 *
 *   It also stores the number of integers in the file (size), the
 *   initial unsorted sequence of integers (*sequence), an array of 
 *   pointers to all the subsequences (**subsequences), another array 
 *   of integers to store all the subsequences length (*subsequences_length)
 *   and the size of subsequences (all_subsequences_size).
 * 
 */
struct File {
  char *filename;
  FILE *file;
  int size;
  int *sequence;
  int **subsequences;
  int *subsequences_length;
  int all_subsequences_size;
};


/**
 *  \brief Read the file.
 *
 *  Reads a binary file and stores its content in an array of integers.
 *
 */
extern void read_file(struct File *file);

/**
 *  \brief Validation of final sequence.
 *
 *  Checks whether the final sequence is properly sorted or not.
 * 
 *  \param file contains the struct File that have all the information needed
 *
 */
extern void validate(struct File *file);


/**
 *  \brief Sort a sequence.
 *
 *  Operation carried out by the workers.
 *
 *  \param subsequence contains the subsequence of integers that needs to be sorted
 *  \param size contains the size of the subsequence
 */
extern int * sort_sequence(int *subsequence, int size);

/**
 *  \brief Merge two sequences.
 *
 *  Operation carried out by the workers. 
 *
 *  \param subsequence1 contains the first subsequence1 of integers that needs to be merged
 *  \param size1 contains the size of the subsequence1
 *  \param subsequence2 contains the second subsequence of integers that needs to be merged
 *  \param size2 contains the size of the subsequence2
 */
extern int * merge_sequences(int *subsequence1, int size1, int *subsequence2, int size2);


/**
 *  \brief Divide the work between the workers.
 *
 *  Operation carried out by the dispatcher.
 * 
 *  \param file contains the struct File that have all the information needed
 *  \param n contains the number of parts that the sequence needs to be divided
 */
extern void divide_work(struct File *file, int n);

/**
 *  \brief Applies the Bitonic Sort algorithm to a subsequence of integers.
 *
 *  Operation carried out by the workers.
 *
 *  \param val contains the subsequence of integers to be sorted
 *  \param N contains the size of the subsequence
 */
extern void bitonicSort(int *val, int N);

#endif /* MONITOR_H */