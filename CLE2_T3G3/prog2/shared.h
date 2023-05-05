
#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


/**
 *  \brief Structure with the task assigned to a worker.
 *
 *   It stores the worker_id, type of task (sort or merge), index of the sequences to be processed and 
 *   a boolean flag to indicate if the worker is busy or not.
 */
struct Task {
    int worker_id;
    char *type;
    int index_sequence1;
    int index_sequence2;
    bool is_busy;
};


/**
 *  \brief Structure with the filename and file pointer to process.
 *
 *   It also stores the number of integers in the file (size), the
 *   initial unsorted sequence of integers (*sequence) and an array of 
 *   pointers to all the subsequences (**all_subsequences).
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
 */
extern void validate();


/**
 *  \brief Sort a sequence.
 *
 *  Operation carried out by the workers.
 *
 *  \param id contains the id of the sequence to be sorted
 */
extern int * sort_sequence(int *subsequence, int size);

/**
 *  \brief Merge two sequences.
 *
 *  Operation carried out by the workers. 
 *
 *  \param worker_id contains the worker id that was assigned to merge the subsequences
 */
extern int * merge_sequences(int *subsequence1, int size1, int *subsequence2, int size2);


/**
 *  \brief Divide the work between the workers.
 *
 *  Operation carried out by the distributor.
 * 
 *  \param n_workers contains the number of workers
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