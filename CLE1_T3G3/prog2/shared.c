#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "shared.h"


/** \brief distributor threads return status */
extern int distributor_status;

/** \brief worker threads return status array */
extern int *workers_status;

/** \brief storage region */
struct File *file;

/** \brief distributor synchronization point when a worker request work to do */
pthread_cond_t cond;

/** \brief flag to indicate if there is work to be done */
bool work_requested = false;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;


/**
 *  \brief Initialize shared region
 *
 *  Store the file names.
 *
 *  \param filename all file names passed in command argumment
 */
void initialize(char *file_name) {  
  // allocating memory file structs
  file = (struct File*)malloc(sizeof(struct File));

  file->filename = file_name;
  file->file = NULL;
}


void read_file() {

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

    // printf("number of integers = %d\n", file->size);

    file->sequence = (int*)malloc(file->size * sizeof(int));

    // Read the contents of the file
    int num;
    int i = 0;
    while(fread(&num, sizeof(int), 1, file->file) == 1) {
        file->sequence[i] = num;
        i++;
    }

    // just for debug
    /* printf("integers: ");
    print(file->sequence, file->size); */
    
    // Close the file
    fclose(file->file);
}

// função apenas para debug
void print(int *val, int N) {
    for (int j = 0; j < N; j++) {
        printf("%d ", val[j]);
    } 
    printf("\n");
}


void divide_work(int n) {

    file->all_subsequences = (struct SubSequence**)malloc(n * sizeof(struct SubSequence));

    int part_size = file->size / n;
    int remainder = file->size % n;
    int start = 0;
    for (int i = 0; i < n; i++) {
        struct SubSequence *subseq = (struct SubSequence*)malloc(sizeof(struct SubSequence));

        int end = start + part_size + (i < remainder ? 1 : 0);

        subseq->subsequence = (int*)malloc((end - start) * sizeof(int));

        int k = 0;
        for (int j = start; j < end; j++) {
            subseq->subsequence[k] = file->sequence[j];
            k++;
        }

        start = end;
        file->all_subsequences[i] = subseq;
    }

    // for debug
    for (int i = 0; i < n; i++) {
        printf("Sub-sequence %d: ", i + 1);
        print(file->all_subsequences[i]->subsequence, 8);
    }
}



void listen(int id) {
    // enter monitor 
    if ((distributor_status = pthread_mutex_lock(&accessCR)) != 0) {
        errno = distributor_status;           // save error in errno
        distributor_status = EXIT_FAILURE;
        perror("[error] on entering monitor(CF)");
        pthread_exit(NULL);
    }

    while (true) {
        // wait for a work request
        while (!work_requested) {
            if ((distributor_status = pthread_cond_wait(&cond, &accessCR)) != 0) { 
                errno = distributor_status;                                                          /* save error in errno */
                perror ("[error] on waiting for worker's request");
                distributor_status = EXIT_FAILURE;
                pthread_exit (&distributor_status);
            }
        }

        // distribute work
        printf("[distributor] starting distribute work\n");

        int len = sizeof(file->all_subsequences) / sizeof(file->all_subsequences[0]);
        for (int i = 0; i < len; i++) {
            printf("Sub-sequence %d: ", i + 1);
            print(file->all_subsequences[i]->subsequence, 8);
        }


        // reset the work flag
        work_requested = false;
    }

    // exit monitor
    if ((distributor_status = pthread_mutex_unlock(&accessCR)) != 0) {
        errno = distributor_status;           // save error in errno
        distributor_status = EXIT_FAILURE;
        perror("[error] on exting monitor(CF)");
        pthread_exit(NULL);
    }
}


void request_work(int id) {
    // signal the distributor thread that work has been requested
    // enter monitor 
    if ((workers_status[id] = pthread_mutex_lock(&accessCR)) != 0) {
        errno = workers_status[id];           // save error in errno
        workers_status[id] = EXIT_FAILURE;
        perror("[error] on entering monitor(CF)");
        pthread_exit(NULL);
    }

    work_requested = true;
    printf("[worker %d] requesting work\n", id);

    if ((workers_status[id] = pthread_cond_signal (&cond)) != 0){ 
        errno = workers_status[id];           // save error in errno
        perror ("[error] on requesting work to the distributor");
        workers_status[id] = EXIT_FAILURE;
        pthread_exit (&workers_status[id]);
    }

    // exit monitor
    if ((workers_status[id] = pthread_mutex_unlock(&accessCR)) != 0) {
        errno = workers_status[id];           // save error in errno
        workers_status[id] = EXIT_FAILURE;
        perror("[error] on exting monitor(CF)");
        pthread_exit(NULL);
    }
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

void bitonicSort(int *val, int N) {
    bitonicSortRecursive(val, 0, N, 1);
}

/**
 *  \brief Merge two subsequences into a sorted sequence
 *
 *  The sorted_sequence array must be allocated with size = left_size + right_size.
 *  The left and right arrays must be sorted.
 *  After the execution of this method, sorted_sequence will contain the sorted sequence of the left and right arrays.
 *
 *  \param sorted_sequence Empty array with size = left_size + right_size
 *  \param left Array containing a sorted subsequence of integers with size = left_size
 *  \param left_size Size of the left array
 *  \param right Array containing a sorted subsequence of integers with size = right_size
 *  \param right_size Size of the right array
 *
 *  \return void, although, modifies the array arr
 */
void merge(int sorted_sequence[], int left[], int left_size, int right[], int right_size) {
    int i = 0, j = 0, k = 0;

    while (i < left_size && j < right_size) {
        if (left[i] <= right[j]) {
            sorted_sequence[k++] = left[i++];
        } else {
            sorted_sequence[k++] = right[j++];
        }
    }

    while (i < left_size) {
        sorted_sequence[k++] = left[i++];
    }

    while (j < right_size) {
        sorted_sequence[k++] = right[j++];
    }
}


/**
 *  \brief Validate the sort method
 *
 *  Check in the end if the sequence of values is properly sorted
 *
 *  \param val 
 *  \param N 
 */
void validate(int *val, int N) {
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
}