#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <string.h>

#include "countWords.h"

/** \brief number of files to process */
int numFiles;

/** \brief maximum number of bytes per chunk */
int maxBytesPerChunk;

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief print command usage */
static void printUsage (char *cmdName);

void print_results(struct File *file_data);

void reset_struct(struct ChunkData *data);


int main(int argc, char *argv[]) {

  // process command line arguments and set up variables
  int M = 5;                    // number max of files to be processed
  char *filenames[M];           // array with M filenames
  numFiles = 0;                 // number of files to process
  maxBytesPerChunk = 4 * 1000;  // max bytes per chunk (default 4)
  int opt;                      // selected option

  int rank, size;
  int dispatcher = 0;

  // MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // This program requires at least 2 processes 
  if (size < 2) {
    fprintf(stderr, "Requires at least two processes.\n");
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  printf("[rank %d] starting\n", rank);

  if (rank == dispatcher) {

    if (argc < 2) {
      printUsage(argv[0]);
      return 1;
    }

    do {
      switch ((opt = getopt(argc, argv, "hf:m:"))) {
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

        case 'm': // number of max bytes per chunk
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

    // start counting the execution time
    (void) get_delta_time ();


    struct File *file_data = (struct File *)malloc(numFiles * sizeof(struct File)); // allocating memory for numFiles of fileData structs

    for (int i = 0; i < numFiles; i++) {

      struct File *file = (file_data + i);

      // initialize struct File data
      file->nWords = 0;
      file->nWordsA = 0;
      file->nWordsE = 0;
      file->nWordsI = 0;
      file->nWordsO = 0;
      file->nWordsU = 0;
      file->nWordsY = 0;
      file->is_finished = false; 
      file->filename = filenames[i];

      // open the file
      FILE *f = fopen(file->filename, "rb");

      if (f == NULL) {
        printf("[error] could not open the file %s\n",file->filename);
        return EXIT_FAILURE;
      }


      // while the process of all file is not finished
      while ( !file->is_finished ) {

        // Send a chunk of data to each worker process for processing
        for (int worker = 1; worker < size; worker++) {

          // structure that has file's chunk to process and the results of that processing 
          struct ChunkData *chunk_data = (struct ChunkData *)malloc(sizeof(struct ChunkData));
          chunk_data->chunk = (int *)malloc(maxBytesPerChunk * sizeof(int));
          chunk_data->nWords = 0; chunk_data->nWordsA = 0; chunk_data->nWordsE = 0;chunk_data->nWordsI = 0; chunk_data->nWordsO = 0;chunk_data->nWordsU = 0; chunk_data->nWordsY = 0;
          chunk_data->is_finished = false;

          // inform the workers if all work is done (1) or not (0), in this case, still have work to do, so the worker will receive the number 0
          int all_work_done = 0;
          MPI_Send(&all_work_done, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);

          // getting a valid chunk of data (see this function in file countWords.c)
          printf("[rank %d] getting chunk data for worker %d\n", rank, worker);
          get_valid_chunk(chunk_data, f); 

          // send the chunk and the struct ChunkData to workers
          printf("[rank %d] sending chunk data to worker %d\n", rank, worker);
          MPI_Send ((char *) chunk_data, sizeof(struct ChunkData), MPI_BYTE, worker, 0, MPI_COMM_WORLD); // MPI_Send with request

          printf("[rank %d] sending list of integers to worker %d\n", rank, worker);
          MPI_Send (chunk_data->chunk, chunk_data->chunk_size, MPI_INT, worker, 0, MPI_COMM_WORLD);      // the chunk buffer

          if (chunk_data->is_finished) {
            fclose(f); // close the file pointer
            file->is_finished = true;
          }
        }

        // struct to save the partial results of the each worker
        struct ChunkData *partial_results = (struct ChunkData *)malloc(sizeof(struct ChunkData));
        for (int worker = 1; worker < size; worker++) { 
  
          // recieve the partial results
          MPI_Recv ((char *) partial_results, sizeof (struct ChunkData), MPI_BYTE, worker, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          printf("[rank %d] saving partial results from worker %d\n", rank, worker);

          // update final results
          file->nWords  += partial_results->nWords;
          file->nWordsA += partial_results->nWordsA;
          file->nWordsE += partial_results->nWordsE;
          file->nWordsI += partial_results->nWordsI;
          file->nWordsO += partial_results->nWordsO;
          file->nWordsU += partial_results->nWordsU;
          file->nWordsY += partial_results->nWordsY;
        }

        free(partial_results);
      }

    }

    // inform the workers if all work is done (1) 
    int all_work_done = 1;
    for (int worker = 1;  worker < size; worker++) {
      MPI_Send(&all_work_done, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
      printf("[rank %d] transmitted message: 'All work done!' to worker %d\n", rank, worker);
    }

    // print the results
    printf("[rank %d] printing results\n", rank);
    print_results(file_data);

    float exec_time = get_delta_time();
    printf("Execution time = %.6fs\n", exec_time);

    free(file_data);
   
  
  } else {
    struct ChunkData *chunk_data = (struct ChunkData *)malloc(sizeof(struct ChunkData));

    printf("[rank %d] waiting for work...\n", rank);
    while (true) {

      // check if there is still work to do 
      int all_work_done = 0;
      MPI_Recv(&all_work_done, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if (all_work_done == 1) {
        printf("[rank %d] received message: All work done!\n", rank);
        free(chunk_data->chunk);
        free(chunk_data);
        break;
      }
    
      // receive the chunk size and then the chunk (list of integers)
      MPI_Recv ((char *) chunk_data, sizeof (struct ChunkData), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("[rank %d] received chunk data from dispatcher!\n", rank);

      int *chunk = (int *)malloc(chunk_data->chunk_size * sizeof(int));
      MPI_Recv(chunk, chunk_data->chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("[rank %d] received list of integers from dispatcher!\n", rank);

      chunk_data->chunk = chunk;

      if (chunk_data->chunk_size != 0){

        // process the chunk of data (see this function in file countWords.c)
        printf("[rank %d] counting words!\n", rank);
        count_words(chunk_data);
      }

      // send the partial results to dispatcher
      printf("[rank %d] sending partial results to dispatcher\n", rank);
      MPI_Send ((char *) chunk_data, sizeof(struct ChunkData), MPI_BYTE, 0, 0, MPI_COMM_WORLD); // MPI_Send with request

      // reset structures
      reset_struct(chunk_data);
    }
  }

  MPI_Finalize();
  
  return EXIT_SUCCESS;
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
           "  -m BytesChunk  --- set the number of bytes per chunk (default: 4)\n"
           "  -h             --- print this help\n", cmdName);
}

/**
 *  \brief Print results of the text processing.
 */
void print_results(struct File *file_data) {
  // printing the results
  for (int i = 0; i < numFiles; i++) {
    printf("\n");
    printf("File name: %s\n", (file_data + i)->filename);
    printf("Total number of words = %d\n", (file_data + i)->nWords);
    printf("N. of words with an\n");
    printf("%7s %7s %7s %7s %7s %7s\n", "A", "E", "I", "O", "U", "Y");
    printf("%7d %7d %7d %7d %7d %7d\n\n", (file_data + i)->nWordsA, (file_data + i)->nWordsE, (file_data + i)->nWordsI, (file_data + i)->nWordsO, (file_data + i)->nWordsU, (file_data + i)->nWordsY);
  }

}


/**
 *  \brief Reset the variables of a structure.
 */
void reset_struct(struct ChunkData *data) {
  // reset struct variables
  int new_size = data->chunk_size;
  data->is_finished = false;
  data->chunk_size = 0;
  data->nWords = 0; data->nWordsA = 0; data->nWordsE = 0; data->nWordsI = 0; data->nWordsO = 0; data->nWordsU = 0; data->nWordsY = 0;
  memset(data->chunk, 0, new_size * sizeof(unsigned int));
}