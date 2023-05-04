
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
 *  \brief Structure that contains a subsequence of integers.
 *
 *   It also stores the size of the subsequence and two booleans:
 *   one to check if it was already sorted and the other to check
 *   if it is being processed.
 */
struct SubSequence {
  unsigned int *subsequence;
  unsigned int size;
  bool is_sorted;
  bool is_being_processed;
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
  unsigned int *sequence;
  struct SubSequence **all_subsequences;
  int all_subsequences_length;
};


/**
 *  \brief Initialization of the data transfer region.
 *
 *  Allocates the memory for an array of structures with the files passed
 *  as argument and initializes it with their names.
 *
 *  \param filename contains the names of the files to be stored
 */
extern void initialize(char *filename, int n_workers);

/**
 *  \brief Read the file.
 *
 *  Reads a binary file and stores its content in an array of integers.
 *
 */
extern void read_file();

/**
 *  \brief Validation of final sequence.
 *
 *  Checks whether the final sequence is properly sorted or not.
 *
 */
extern void validate();

/**
 *  \brief Request for work.
 *
 *  Operation carried out by the workers.
 */
extern void request_work();

/**
 *  \brief Sort a sequence.
 *
 *  Operation carried out by the workers.
 *
 *  \param id contains the id of the sequence to be sorted
 */
extern void sort_sequence(int id);

/**
 *  \brief Merge two sequences.
 *
 *  Operation carried out by the workers. 
 *
 *  \param worker_id contains the worker id that was assigned to merge the subsequences
 */
extern void merge_sequences(int worker_id);

/**
 *  \brief Notify the distributor that work has been completed.
 *
 *  Operation carried out by the workers.
 *
 *  \param id contains the worker id that has completed its task
 */
extern void notify(int id);

/**
 *  \brief Divide the work between the workers.
 *
 *  Operation carried out by the distributor.
 * 
 *  \param n_workers contains the number of workers
 */
extern void divide_work(int n_workers);

/**
 *  \brief Listening for workers' activity and handling their requests.
 *
 *  Operation carried out by the distributor.
 * 
 *  \param n_workers contains the number of workers
 */
extern void listen(int n_workers);

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