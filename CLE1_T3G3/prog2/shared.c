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


/** \brief distributor threads return status */
extern int distributor_status;

/** \brief worker threads return status array */
extern int *workers_status;

/** \brief queue containing the workers that requested for work by order of arrival */
extern int *waiting_work_queue;

/** \brief storage region */
struct File *file;

/** \brief distributor synchronization point when a worker request work to do */
pthread_cond_t work_request_cond;

/** \brief distributor synchronization point when a worker finishes its assigned work */
pthread_cond_t work_done_cond;

/** \brief flag to indicate if there is work to be done */
bool work_requested;

/** \brief last index of waiting queue */
int index_waiting_queue;

/** \brief check if queue is empty */
bool empty_queue = true;

/** \brief array of tasks assigned to the workers */
struct Task *tasks;

/** \brief bool that is true if all work is done, false otherwise */
extern bool all_work_done;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;


/**
 *  \brief Initialize shared region
 *
 *  Store the file names.
 *
 *  \param filename all file names passed in command argumment
 */
void initialize(char *file_name, int n_workers) {  
  // allocating memory file structs
  file = (struct File*)malloc(sizeof(struct File));

  file->filename = file_name;
  file->file = NULL;

  pthread_cond_init (&work_request_cond, NULL);    // initialize work_request synchronization point 
  pthread_cond_init (&work_done_cond, NULL);       // initialize work_done synchronization point

  work_requested = false;
  index_waiting_queue = 0;
  all_work_done = false;

  // initialize storage struct for tasks 
  tasks = (struct Task *)malloc(n_workers * sizeof(struct Task));

  for (int i = 0; i < n_workers; i++) {
    (tasks + i)->worker_id = i;
    (tasks + i)->is_busy   = false;
  }
}

/**
 *  \brief Read the file.
 *
 *  Reads a binary file and stores its content in an array of integers.
 *
 */
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
 *  \param n_workers contains the number of workers
 */
void divide_work(int n) {

    file->all_subsequences = (struct SubSequence**)malloc(n * sizeof(struct SubSequence));
    file->all_subsequences_length = n;

    int part_size = file->size / n;
    int remainder = file->size % n;
    int start = 0;
    for (int i = 0; i < n; i++) {
        struct SubSequence *subseq = (struct SubSequence*)malloc(sizeof(struct SubSequence));

        int end = start + part_size + (i < remainder ? 1 : 0);

        subseq->subsequence = (int*)malloc((end - start) * sizeof(int));
        subseq->size = (end - start);
        subseq->is_being_processed = false;
        subseq->is_sorted = false;

        int k = 0;
        for (int j = start; j < end; j++) {
            subseq->subsequence[k] = file->sequence[j];
            k++;
        }

        start = end;
        file->all_subsequences[i] = subseq;
    }
}


/**
 *  \brief Listening for workers' activity and handling their requests.
 *
 *  Operation carried out by the distributor.
 * 
 *  \param n_workers contains the number of workers
 */
void listen(int n_workers) {
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
            // in case there are workers in the waiting queue
            if (!empty_queue) break;
            
            if ((distributor_status = pthread_cond_wait(&work_request_cond, &accessCR)) != 0) { 
                errno = distributor_status;                          // save error in errno 
                perror ("[error] on waiting for worker's request");
                distributor_status = EXIT_FAILURE;
                pthread_exit (&distributor_status);
            }
        }     

        // distribute work
        int worker_id = waiting_work_queue[0];      // get the fist worker waiting for work

        // before distributing the sorting work, check if there is a way to assign merge work
        int sorted_subsequences = 0;
        int *subsequences_to_merge = (int*)malloc(2 * sizeof(int));
        for (int i = 0; i < file->all_subsequences_length; i++) {
            if (file->all_subsequences[i]->is_sorted && sorted_subsequences < 2) {
                subsequences_to_merge[sorted_subsequences] = i;
                sorted_subsequences++;  
            } 
        }


        if (sorted_subsequences == 2) {
            // merge
            printf("[distributor] distributes merge task to worker %d\n", worker_id);
            (tasks + worker_id)->type = "merge";
            (tasks + worker_id)->index_sequence1 = subsequences_to_merge[0];   // addresses the subsequence id in all_subsequences
            (tasks + worker_id)->index_sequence2 = subsequences_to_merge[1];   // addresses the subsequence id in all_subsequences
            (tasks + worker_id)->is_busy = true;

        } else { 
            // sort task
            for (int i = 0; i < n_workers; i++) {           
                if (!file->all_subsequences[i]->is_being_processed) {
                    // assign to worker the subsequence file->all_subsequences[i]->subsequence
                    printf("[distributor] distributes sort task of the subsequence %d to worker %d\n", worker_id, worker_id);

                    file->all_subsequences[i]->is_being_processed = true;

                    (tasks + worker_id)->type = "sort";
                    (tasks + worker_id)->index_sequence1 = i;   // addresses the subsequence id in all_subsequences
                    (tasks + worker_id)->is_busy = true;
                    break;
                }
            }
        }

        // reset the work flag
        work_requested = false;

        // Wait for the work_done notification
        if ((distributor_status = pthread_cond_wait(&work_done_cond, &accessCR)) != 0) { 
            errno = distributor_status;                          // save error in errno 
            perror ("[error] on waiting for worker's notification");
            distributor_status = EXIT_FAILURE;
            pthread_exit (&distributor_status);
        }

        printf("[distributor] notification that the work is done received\n");

        // remove first element of waiting_work_queue and decrease the pointer to last element, index_waiting_queue
        int i;
        for (i = 0; i < index_waiting_queue; i++) {
            waiting_work_queue[i] = waiting_work_queue[i + 1];
        }
        waiting_work_queue[i] = -1;
        index_waiting_queue--;
        if (index_waiting_queue == 0) empty_queue = true;
        
        (tasks + worker_id)->is_busy = false;


        if (file->all_subsequences_length == 1) {
            all_work_done = true;
            break;
        }
    }

    // exit monitor
    if ((distributor_status = pthread_mutex_unlock(&accessCR)) != 0) {
        errno = distributor_status;           // save error in errno
        distributor_status = EXIT_FAILURE;
        perror("[error] on exting monitor(CF)");
        pthread_exit(NULL);
    }
}

/**
 *  \brief Request for work.
 *
 *  Operation carried out by the workers.
 */
void request_work(int worker_id) {
    // signal the distributor thread that work has been requested
    // enter monitor 
    if ((workers_status[worker_id] = pthread_mutex_lock(&accessCR)) != 0) {
        errno = workers_status[worker_id];           // save error in errno
        workers_status[worker_id] = EXIT_FAILURE;
        perror("[error] on entering monitor(CF)");
        pthread_exit(NULL);
    }

    printf("[worker %d] requesting work\n", worker_id);
    work_requested = true;
    empty_queue = false;

    waiting_work_queue[index_waiting_queue] = worker_id;
    index_waiting_queue++;
    
    if ((workers_status[worker_id] = pthread_cond_signal (&work_request_cond)) != 0){ 
        errno = workers_status[worker_id];           // save error in errno
        perror ("[error] on requesting work to the distributor");
        workers_status[worker_id] = EXIT_FAILURE;
        pthread_exit (&workers_status[worker_id]);
    }

    // exit monitor
    if ((workers_status[worker_id] = pthread_mutex_unlock(&accessCR)) != 0) {
        errno = workers_status[worker_id];           // save error in errno
        workers_status[worker_id] = EXIT_FAILURE;
        perror("[error] on exting monitor(CF)");
        pthread_exit(NULL);
    }
}

/**
 *  \brief Sort a sequence.
 *
 *  Operation carried out by the workers.
 *
 *  \param id contains the id of the sequence to be sorted
 */
void sort_sequence(int id) {
    // sort sequence

    int subseq_index = (tasks + id)->index_sequence1;
    struct SubSequence *sub_seq = file->all_subsequences[subseq_index];

    bitonicSort(sub_seq->subsequence, sub_seq->size);
    sub_seq->is_sorted = true;
    file->all_subsequences[subseq_index] = sub_seq;

    printf("[worker %d] sorted the sequence!\n", id);
}

/**
 *  \brief Notify the distributor that work has been completed.
 *
 *  Operation carried out by the workers.
 *
 *  \param id contains the worker id that has completed its task
 */
void notify(int id) {
    // Signal the distributor thread that work has been finished
    // Enter monitor
    if ((workers_status[id] = pthread_mutex_lock(&accessCR)) != 0) {
        errno = workers_status[id];           // save error in errno
        workers_status[id] = EXIT_FAILURE;
        perror("[error] on entering monitor(CF)");
        pthread_exit(NULL);
    }

    printf("[worker %d] notifying that work is done\n", id);

    if ((workers_status[id] = pthread_cond_signal(&work_done_cond)) != 0) { 
        errno = workers_status[id];           // save error in errno
        perror("[error] on notifying distributor that work is done");
        workers_status[id] = EXIT_FAILURE;
        pthread_exit(&workers_status[id]);
    }

    // Exit monitor
    if ((workers_status[id] = pthread_mutex_unlock(&accessCR)) != 0) {
        errno = workers_status[id];           // save error in errno
        workers_status[id] = EXIT_FAILURE;
        perror("[error] on exiting monitor(CF)");
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
void merge_sequences(int worker_id) {

    int index_subsequence1 = (tasks + worker_id)->index_sequence1;
    int index_subsequence2 = (tasks + worker_id)->index_sequence2;

    int *left = file->all_subsequences[index_subsequence1]->subsequence;
    int left_size = file->all_subsequences[index_subsequence1]->size;

    int *right = file->all_subsequences[index_subsequence2]->subsequence;
    int right_size = file->all_subsequences[index_subsequence2]->size;

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

    printf("[worker %d] merge done\n", worker_id);

    int new_size = file->all_subsequences_length - 1;
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
    file->all_subsequences_length = new_size;

}


/**
 *  \brief Validate the sort method
 *
 *  Check in the end if the sequence of values is properly sorted
 *
 */
void validate() {

    printf ("\n");

    int *val = file->all_subsequences[0]->subsequence;
    int N    = file->all_subsequences[0]->size;

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