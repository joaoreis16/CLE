/**
 *  \file shared.c (implementation file)
 *
 *  Synchronization based on monitors.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  This shared region will use the array of structures initialized by
 *  the main thread.
 * 
 *  Workers can access the shared region to obtain subsequences to process from that
 *  array of structures.
 * 
 *  There is also a function (validate) to check whether the resultant sequence is correctly sorted, 
 *  which is used when there is no more work to be carried out.
 * 
 *  \brief Role of the main thread 
 * 
 *   1. to get the text file names by processing the command line and storing them in 
 *   the shared region
 *
 *   2. to create the distributor and worker threads and wait for their termination
 *
 *   3. to print the validation of the resultant sorted sequence.
 *
 *  \author Artur Romão e João Reis - March 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "shared.h"



/**
 *  \brief Read the file.
 *
 *  Reads a binary file and stores its content in an array of integers.
 *
 */
void read_file(struct File *file) {
    // Open binary file for reading
    file->file = fopen(file->filename, "rb");
    
    if (file->file == NULL) {
        printf("Error opening the file\n");
        exit(EXIT_FAILURE);
    }

    // Read the header of the binary file
    if (fread(&file->size, sizeof(int), 1, file->file) != 1) {
        printf("Error reading the file\n");
        exit(EXIT_FAILURE);
    }

    file->sequence = (int*)malloc(file->size * sizeof(int));

    // Read the contents of the file
    int num;
    int i = 0;
    while(fread(&num, sizeof(int), 1, file->file) == 1) {
        file->sequence[i] = num;
        i++;
    }
   
    // Close the file
    fclose(file->file);
}

/**
 *  \brief Divide the work between the workers.
 *
 *  Operation carried out by the distributor.
 * 
 *  \param n contains the number of workers
 */
void divide_work(struct File *file, int n) {

    file->subsequences = (int **)malloc(n * sizeof(int *));
    file->subsequences_length = (int *)malloc(n * sizeof(int));

    int part_size = file->size / n;
    
    int remainder = file->size % n;
    int start = 0;
    for (int i = 0; i < n; i++) {
        int end = start + part_size + (i < remainder ? 1 : 0);

        int *subsequence = (int *)malloc((end - start) * sizeof(int));
        int size = (end - start);

        int k = 0;
        for (int j = start; j < end; j++) {
            subsequence[k] = file->sequence[j];
            k++;
        }

        start = end;
        file->subsequences[i] = subsequence;
        file->subsequences_length[i] = size;
    }

    file->all_subsequences_size = n;
}



/**
 *  \brief Sort a sequence.
 *
 *  Operation carried out by the workers.
 *
 *  \param id contains the id of the sequence to be sorted
 */
int * sort_sequence(int *subsequence, int size) {
    bitonicSort(subsequence, size);
    return subsequence;
}

void compareAndPossibleSwap(int *val, int i, int j, int dir) {
    if ((val[i] > val[j]) == dir) {
        int temp = val[i];
        val[i] = val[j];
        val[j] = temp;
    }
}

void bitonicMerge(int *val, int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; i++) {
            compareAndPossibleSwap(val, i, i + k, dir);
        }
        bitonicMerge(val, low, k, dir);
        bitonicMerge(val, low + k, k, dir);
    }
}

void bitonicSortRecursive(int *val, int low, int cnt, int dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonicSortRecursive(val, low, k, !dir);
        bitonicSortRecursive(val, low + k, k, dir);
        bitonicMerge(val, low, cnt, dir);
    }
}

/**
 *  \brief Applies the Bitonic Sort algorithm to a subsequence of integers.
 *
 *  Operation carried out by the workers.
 *
 *  \param val contains the subsequence of integers to be sorted
 *  \param N contains the size of the subsequence
 */
void bitonicSort(int *val, int N) {
    bitonicSortRecursive(val, 0, N, 1);
}


/**
 *  \brief Merge two sequences.
 *
 *  Operation carried out by the workers. 
 *
 *  \param worker_id contains the worker id that was assigned to merge the subsequences
 */
int * merge_sequences(int *subsequence1, int size1, int *subsequence2, int size2) {

    int *left = subsequence1;
    int left_size = size1;

    int *right = subsequence2;
    int right_size = size2;

    int *merged_subsequence = (int*)malloc((left_size + right_size) * sizeof(int));

    int i = 0, j = 0, k = 0;

    while (i < left_size && j < right_size) {
        if (left[i] <= right[j]) {
            merged_subsequence[k++] = left[i++];
        } else {
            merged_subsequence[k++] = right[j++];
        }
    }

    while (i < left_size) {
        merged_subsequence[k++] = left[i++];
    }

    while (j < right_size) {
        merged_subsequence[k++] = right[j++];
    }

    return merged_subsequence;


    /* int new_size = file->all_subsequences_length - 1;
    struct SubSequence **new_all_subseqs = (struct SubSequence**)malloc(new_size * sizeof(struct SubSequence *));
    
    int idx = 0;
    bool put_merged_subsequence = false;
    for (int i = 0; i < file->all_subsequences_length; i++) {
        if ((i == index_subsequence1 || i == index_subsequence2)) {
            if (!put_merged_subsequence) {            
                struct SubSequence *subseq = (struct SubSequence*)malloc(sizeof(struct SubSequence));
                subseq->subsequence = merged_subsequence;
                subseq->size = left_size + right_size;
                subseq->is_sorted = true;
                subseq->is_being_processed = true;
                new_all_subseqs[idx++] = subseq;
                put_merged_subsequence = true;
            }
        } else {  // copy the subsequence
            new_all_subseqs[idx++] = file->all_subsequences[i];
        }
    }

    for (int i = 0; i < file->all_subsequences_length; i++) {
        if (i == index_subsequence1 || i == index_subsequence2) {
            free(file->all_subsequences[i]);
        }
    }
    free(file->all_subsequences);
    
    file->all_subsequences = new_all_subseqs;
    file->all_subsequences_length = new_size; */

}