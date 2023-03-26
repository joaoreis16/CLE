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

/** \brief  */
extern int *waiting_work_queue;

/** \brief  */
extern int *work_assignment;

/** \brief storage region */
struct File *file;

/** \brief distributor synchronization point when a worker request work to do */
pthread_cond_t work_request_cond;

/** \brief distributor synchronization point when a worker finishes its assigned work */
pthread_cond_t work_done_cond;

/** \brief flag to indicate if there is work to be done */
bool work_requested = false;

/** \brief last index of waiting queue */
int index_waiting_queue = 0;

/** \brief  */
bool empty_queue = true;

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

  pthread_cond_init (&work_request_cond, NULL);    // initialize work_request synchronization point 
  pthread_cond_init (&work_done_cond, NULL);       // initialize work_done synchronization point
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
        subseq->size = (end - start);
        subseq->is_being_sorted = false;
        subseq->is_sorted = false;

        int k = 0;
        for (int j = start; j < end; j++) {
            subseq->subsequence[k] = file->sequence[j];
            k++;
        }

        start = end;
        file->all_subsequences[i] = subseq;
    }

    // for debug
    /* for (int i = 0; i < n; i++) {
        printf("Sub-sequence %d: ", i + 1);
        print(file->all_subsequences[i]->subsequence, 8);
    } */
}



void listen(int id, int n_workers) {
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
            // caso haja workers na fila de espera
            if (!empty_queue) break;    // 

            if ((distributor_status = pthread_cond_wait(&work_request_cond, &accessCR)) != 0) { 
                errno = distributor_status;                          // save error in errno 
                perror ("[error] on waiting for worker's request");
                distributor_status = EXIT_FAILURE;
                pthread_exit (&distributor_status);
            }
        }     

        // distribute work
        printf("waiting queue: ");
        print(waiting_work_queue, n_workers);

        int worker_id = waiting_work_queue[0];      // get the fist worker waiting for work


        // antes de distribuir o trabalho de ordenação, vê se dá para atribuir trabalho de merge
        int sorted_subsequences = 0;
        int *subsequences_to_merge = (int*)malloc(2 * sizeof(int));
        for (int i = 0; i < n_workers; i++) {
            if (file->all_subsequences[i]->is_sorted){
              
              sorted_subsequences++;  
            } 
        }

        if (sorted_subsequences >= 2) {
            // fazer merge
            
            
            
        } else {
            for (int i = 0; i < n_workers; i++) {           
                if (!file->all_subsequences[i]->is_being_sorted) {
                    // atribuir ao worker a sequencia file->all_subsequences[i]->subsequence
                    printf("[distributor] distributes subsequence %d to worker %d\n", i, worker_id);
                    // print(file->all_subsequences[i]->subsequence, 8);

                    file->all_subsequences[i]->is_being_sorted = true;
                    work_assignment[worker_id] = i;     // addresses the subsequence id in all_subsequences
                    // print(work_assignment, n_workers);
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


        // Check if all work is done and break the loop if necessary
        int all_subsequences_sorted = 0;
        for (int i = 0; i < n_workers; i++) {
            if (file->all_subsequences[i]->is_sorted) {
                all_subsequences_sorted++;
            }
        }
        
        
        // final if -> 1 remaining subsequence (that is sorted); break;
        if (all_subsequences_sorted == n_workers) {
            printf(">> All sub-sequences sorted!\n");
            // break;
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
    // printf("index_waiting_queue: %d  | worker_id: %d\n", index_waiting_queue, worker_id);
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

void sort_sequence(int id) {
    // sort sequence
    int subseq_index = work_assignment[id];
    struct SubSequence *sub_seq = file->all_subsequences[subseq_index];

    // print de todos as subsequences->is_sorted
    /* printf("antes de uma sequencia ficar sorted: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", file->all_subsequences[i]->is_sorted);
    }
    printf("\n"); */

    bitonicSort(sub_seq->subsequence, sub_seq->size);
    sub_seq->is_sorted = true;
    file->all_subsequences[subseq_index] = sub_seq;

    /* printf("depois de uma sequencia ficar sorted: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", file->all_subsequences[i]->is_sorted);
    }
    printf("\n"); */

    printf("[worker %d] sorted the sequence!\n", id);

}


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