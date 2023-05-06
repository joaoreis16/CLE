#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "sortInt.h"


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
 *  Operation carried out by the dispatcher.
 * 
 *  \param file contains the struct File that have all the information needed
 *  \param n contains the number of parts that the sequence needs to be divided
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
 *  \param subsequence contains the subsequence of integers that needs to be sorted
 *  \param size contains the size of the subsequence
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
    int size = 1;
    while (size < N) {
        size <<= 1;
    }

    int *padded_val = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < N; i++) {
        padded_val[i] = val[i];
    }
    for (int i = N; i < size; i++) {
        padded_val[i] = INT_MAX;
    }

    bitonicSortRecursive(padded_val, 0, size, 1);

    for (int i = 0; i < N; i++) {
        val[i] = padded_val[i];
    }

    free(padded_val);
}


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
}


/**
 *  \brief Validation of final sequence.
 *
 *  Checks whether the final sequence is properly sorted or not.
 * 
 *  \param file contains the struct File that have all the information needed
 *
 */
void validate(struct File *file) {

    int *val = file->sequence;
    int N    = file->size;

    int i;
    for (i = 0; i < N; i++) {

        if (i == (N - 1))  {
            printf ("Everything is OK!\n");
            break;
        }

        if (val[i] > val[i+1]) { 
            printf ("Error in position %d between element %d and %d\n", i, val[i], val[i+1]);
            break;
        }
    }

    printf ("\n");
}