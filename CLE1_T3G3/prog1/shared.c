/**
 *  \file shared.c (implementation file)
 *
 *  \brief Memory Shared Region for the text processing problem with multithreading.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  This shared region will utilize the array of structures initialized by
 *  the main thread.
 * 
 *  Workers can access the shared region to obtain data to process from that
 *  array of structures. They can also store the partial results of the
 *  processing done.
 * 
 *  There is also a function to print out the final results, that should be
 *  used after there is no more data to be processed.
 * 
 *  Monitored Methods:
 *     \li getData - operation carried out by worker threads.
 *     \li savePartialResults - operation carried out by worker threads.
 *
 *  Unmonitored Methods:
 *     \li putInitialData - operation carried out by the main thread.
 *     \li printResults - operation carried out by the main thread.
 *
 *  \author Artur Romão e João Reis - March 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "shared.h"
#include "countWords.h"

/** \brief status array of workers (0 if idle, 1 if working) */
extern int *workers_status;

/** \brief number of files that will be read and process */
extern int numFiles;

/** \brief max number of bytes per chunk */
extern int maxBytesPerChunk;

/** \brief if all work is done */
extern bool all_work_done;

/** \brief storage region */
struct File *file_data;

/** \brief current file index being processed */
static int file_index = 0;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/**
 *  \brief Initialize shared region
 *
 *  Store the file names.
 *
 *  \param filenames all file names passed in command argumment
 */
void initialize(char *filenames[]) {  
  // allocating memory for numFiles of file structs
  file_data = (struct File *)malloc(numFiles * sizeof(struct File));

  for (int i = 0; i < numFiles; i++) {
    (file_data + i)->file_name = filenames[i];
    (file_data + i)->file = NULL;

    file_data->nWords  = 0;
    file_data->nWordsA = 0;
    file_data->nWordsE = 0;
    file_data->nWordsI = 0;
    file_data->nWordsO = 0;
    file_data->nWordsU = 0;
    file_data->nWordsY = 0;
  }
}

/**
 *  \brief Proce process from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param id worker identification
 *  \param data structure that will store the chunk of chars to process
 */
void get_chunk(unsigned int id, struct ChunkData *data) {
  // enter monitor 
  if ((workers_status[id] = pthread_mutex_lock(&accessCR)) != 0) {
    errno = workers_status[id];           // save error in errno
    workers_status[id] = EXIT_FAILURE;
    perror("[error] on entering monitor(CF)");
    pthread_exit(NULL);
  }
  printf("\n[MUTEX] locked\n");
  printf(">> (%d) Getting chunk...\n", id);

  if (!all_work_done) {

    struct File *actual_file = (file_data + file_index);

    // if file hasn't been open yet 
    if (actual_file->file == NULL) {

      actual_file->file = fopen(actual_file->file_name, "rb");
      if (actual_file->file == NULL) {
        printf("[error] could not open the file %s\n", actual_file->file_name);
        exit(EXIT_FAILURE);
      }
    }

    data->is_finished = false; 
    data->index = file_index;              // file index on the shared region array structure

    printf(">> (%d) Getting valid chunk...\n", id);
    get_valid_chunk(data, actual_file);

    if (data->is_finished) {
      // avançar para o próximo ficheiro
      file_index++;
      fclose(actual_file->file); // close the file pointer
      
      // ou dizer ao próximo worker que já não há benfica trabalhar
      if (numFiles == file_index) {
        printf(">> All work done!\n");
        all_work_done = true;
      }
    }
  }

  // exit monitor
  if ((workers_status[id] = pthread_mutex_unlock(&accessCR)) != 0) {
    errno = workers_status[id];           // save error in errno
    workers_status[id] = EXIT_FAILURE;
    perror("[error] on exting monitor(CF)");
    pthread_exit(NULL);
  }
  printf("[MUTEX] unlocked\n\n");
}


void process_chunk(unsigned int id, struct ChunkData *data) {
  printf(">> (%d) Processing chunk...\n", id);
  count_words(data);
}

void update_counters(unsigned int id, struct ChunkData *data) {
  // enter monitor 
  if ((workers_status[id] = pthread_mutex_lock(&accessCR)) != 0) {
    errno = workers_status[id];           // save error in errno
    workers_status[id] = EXIT_FAILURE;
    perror("[error] on entering monitor(CF)");
    pthread_exit(NULL);
  }
  printf("\n[MUTEX] locked\n");
  printf(">> (%d) Updating counters...\n", id);

  // update counters
  (file_data + data->index)->nWords  += data->nWords;
  (file_data + data->index)->nWordsA += data->nWordsA;
  (file_data + data->index)->nWordsE += data->nWordsE;
  (file_data + data->index)->nWordsI += data->nWordsI;
  (file_data + data->index)->nWordsO += data->nWordsO;
  (file_data + data->index)->nWordsU += data->nWordsU;
  (file_data + data->index)->nWordsY += data->nWordsY;

  // exit monitor
  if ((workers_status[id] = pthread_mutex_unlock(&accessCR)) != 0) {
    errno = workers_status[id];           // save error in errno
    workers_status[id] = EXIT_FAILURE;
    perror("[error] on exting monitor(CF)");
    pthread_exit(NULL);
  }
  printf("[MUTEX] unlocked\n\n");
}


void print_results() {
  // printing the results
  for (int i = 0; i < numFiles; i++) {
    printf("\n");
    printf("File name: %s\n", (file_data + i)->file_name);
    printf("Total number of words = %d\n", (file_data + i)->nWords);
    printf("N. of words with an\n");
    printf("%7s %7s %7s %7s %7s %7s\n", "A", "E", "I", "O", "U", "Y");
    printf("%7d %7d %7d %7d %7d %7d\n\n", (file_data + i)->nWordsA, (file_data + i)->nWordsE, (file_data + i)->nWordsI, (file_data + i)->nWordsO, (file_data + i)->nWordsU, (file_data + i)->nWordsY);
  }

}