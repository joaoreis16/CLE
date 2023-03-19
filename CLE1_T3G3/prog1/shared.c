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

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief storage region */
struct File *file_data;

/** \brief current file index being processed */
static int file_index = 0;


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
  if (pthread_mutex_lock(&accessCR) != 0) {
    workers_status[id] = -1;
    perror("[error] on entering monitor(CF)");
    pthread_exit(NULL);
  }

  workers_status[id] = 1;
  printf("\n\n>> Starting worker thread with id = %d\n", id);

  struct File *file = (file_data + file_index);

  file->file = fopen(file->file_name, "rb");
  if (file->file == NULL) {
    printf("[error] could not open the file %s\n", file->file_name);
    exit(EXIT_FAILURE);
  }

  data->is_finished = false; 
  // data->chunk_size = fread(data->chunk, 1, maxBytesPerChunk-7, file->file); // maxBytesPerChunk - 7 


  // current file has reached the end
  /* 
  if (data->chunk_size < maxBytesPerChunk) {
    file_index++;              // update the current file being processed index 
    fclose(file->file);

  } else {

  } */
   

  get_valid_chunk(data, file);

  // exit monitor
  if (pthread_mutex_unlock(&accessCR) != 0) {
    workers_status[id] = -1;
    perror("[error] on exting monitor(CF)");
    pthread_exit(NULL);
  }
  workers_status[id] = 0;
}