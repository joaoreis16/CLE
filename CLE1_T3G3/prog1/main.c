/**
 *  \file main.c
 *
 *  \brief Problem name: Text Processing with Multithreading.
 *
 *  The main objective of this program is to process files in order to obtain
 *  the number of words, and the number of words containing with a vowel.
 *
 *  It is optimized by splitting the work between worker threads which after obtaining
 *  the chunk of the file from the shared region, perform the calculations and then save
 *  the processing results.
 *
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  \author Artur Romão e João Reis - March 2023
 */

/** 
 * \brief Role of the main thread 
 * 
 *   1. to get the text file names by processing the command line and storing them in 
 *   the shared region
 *
 *   2. to create the worker threads and wait for their termination
 *
 *   3. to print the results of the processing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#include "shared.h"

/** \brief worker threads return status array */
int *workers_status;

/** \brief number of files to process */
int numFiles;

/** \brief maximum number of bytes per chunk */
int maxBytesPerChunk;

/** \brief worker life cycle routine */
static void *worker (void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief print command usage */
static void printUsage (char *cmdName);


int main(int argc, char *argv[]) {

  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }

  // process command line arguments and set up variables
  int n_workers = 4;            // number of worker threads
  int M = 5;                    // number max of files to be processed
  char *filenames[M];           // array with M filenames
  numFiles = 0;                 // number of files to process
  maxBytesPerChunk = 4 * 1000;  // max bytes per chunk (default 4)
  int opt;                      // selected option

  do {
    switch ((opt = getopt(argc, argv, "hf:w:m:"))) {
      case 'f': // file name
        if (optarg[0] == '-') {
          fprintf(stderr, "%s: file name is missing\n", argv[0]);
          printUsage(argv[0]);
          return EXIT_FAILURE;
        }
        if (numFiles == M) {
          fprintf(stderr, "%s: can only process %d files at a time\n", argv[0], M);
          return EXIT_FAILURE;
        }
        filenames[numFiles++] = optarg;
        break;

      case 'w': // n. of workers
        if (atoi(optarg) < 1) {
          fprintf(stderr, "%s: number of worker threads must be greater or equal than 1\n", argv[0]);
          printUsage(argv[0]);
          return EXIT_FAILURE;
        }
        n_workers = (int)atoi(optarg);
        break;

      case 'm': // n. of max bytes per chunk
        if (atoi(optarg) != 4 && atoi(optarg) != 8) {
          fprintf(stderr, "%s: number of bytes must be 4 or 8 kBytes\n", argv[0]);
          printUsage(argv[0]);
          return EXIT_FAILURE;
        }
        maxBytesPerChunk = (int)atoi(optarg) * 1000;
        break;

      case 'h': // help mode
        printUsage(argv[0]);
        return EXIT_SUCCESS;

      case '?': // invalid option
        fprintf(stderr, "%s: invalid option\n", argv[0]);
        printUsage(argv[0]);
        return EXIT_FAILURE;

      case -1:
        break;
    }

  } while (opt != -1);

  // storing file names in the shared region
  initialize(filenames);

  workers_status = malloc(sizeof(int) * n_workers);
  pthread_t *pthread_workers;         // workers internal thread id array (alterar este comment)
  unsigned int *workers;              // workers application defined thread id array 
  int *pStatus;                       // pointer to execution status 

  if (((pthread_workers = malloc (n_workers * sizeof (pthread_t))) == NULL) || ((workers = malloc (n_workers * sizeof (int))) == NULL)) { 
    fprintf (stderr, "[error] on allocating space to both internal / external producer / consumer id arrays\n");
    return EXIT_FAILURE;
  }

  // start counting the execution time
  (void) get_delta_time ();

  // creating worker threads 
  for (int i = 0; i < n_workers; i++) {
    workers[i] = i;   // add new worker with ID i

    if (pthread_create(&pthread_workers[i], NULL, worker, &workers[i]) != 0) {
      perror("[error] on creating thread worker");
      return EXIT_FAILURE;
    }
  }
 
  // waiting for the termination of the worker threads
  for (int i = 0; i < n_workers; i++) {
    if (pthread_join(pthread_workers[i], (void *)&pStatus) != 0) {
      perror("[error] on waiting for worker thread");
      return EXIT_FAILURE;
    }
  }

  float exec_time = get_delta_time();
  printf ("\nExecution time = %.6fs\n", exec_time);

  return EXIT_SUCCESS;
}


/**
 *  \brief Function producer.
 *
 *  While there is work to be carried out
 *     − to request chunks of text of some text file
 *     − to process them
 *     − to return the partial results.
 *
 *  \param worker_id pointer to application defined producer identification
 */
static void *worker (void *worker_id) {
  unsigned int id = *((unsigned int *)worker_id); // worker id

  // structure that has file's chunk to process and the results of that processing 
  struct ChunkData *chunk_data = (struct ChunkData *)malloc(sizeof(struct ChunkData));
  chunk_data->chunk = (unsigned char *)malloc(maxBytesPerChunk * sizeof(unsigned char));

  // get result of the chunk processing
  get_chunk(id, chunk_data); 

} 

/**
 *  \brief Get the process time that has elapsed since last call of this time.
 *
 *  \return process elapsed time
 */
static double get_delta_time(void) {
  static struct timespec t0, t1;

  t0 = t1;
  if(clock_gettime (CLOCK_MONOTONIC, &t1) != 0) {
    perror ("clock_gettime");
    exit(1);
  }
  return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}


/**
 *  \brief Print command usage.
 *
 *  A message specifying how the program should be called is printed.
 *
 *  \param cmdName string with the name of the command
 */

static void printUsage(char *cmdName) {
  fprintf (stderr, "\nSynopsis: %s [OPTIONS]\n"
           "  OPTIONS:\n"
           "  -f filename    --- set the file name (max usage: 5)\n"
           "  -w nWorkers    --- set the number of workers (default: 4)\n"
           "  -m BytesChunk  --- set the number of bytes per chunk (default: 4)\n"
           "  -h             --- print this help\n", cmdName);
}