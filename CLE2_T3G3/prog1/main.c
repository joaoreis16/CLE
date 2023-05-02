#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#include "countWords.h"

/** \brief worker threads return status array */
int *workers_status;

/** \brief number of files to process */
int numFiles;

/** \brief maximum number of bytes per chunk */
int maxBytesPerChunk;

/** \brief bool that is true if all work is done, false otherwise */
bool all_work_done;

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief print command usage */
static void printUsage (char *cmdName);

void get_chunk(struct ChunkData *data, FILE *file);

int main(int argc, char *argv[]) {

  // process command line arguments and set up variables
  int M = 5;                    // number max of files to be processed
  char *filenames[M];           // array with M filenames
  numFiles = 0;                 // number of files to process
  maxBytesPerChunk = 4 * 1000;  // max bytes per chunk (default 4)
  int opt;                      // selected option
  all_work_done = false;        // if all work is done 

  int rank, size;
  int dispatcher = 0;

  // MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Request request = MPI_REQUEST_NULL;
  MPI_Status status;

  /* This program requires at least 2 processes */
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


    struct File *file_data = (struct File *)malloc(numFiles * sizeof(struct File)); /* allocating memory for numFiles of fileData structs */

    for (int i = 0; i < numFiles; i++) {

      struct File *file = (file_data + i);

      /* initialize struct data */
      file->nWords = 0;
      file->nWordsA = 0;
      file->nWordsE = 0;
      file->nWordsI = 0;
      file->nWordsO = 0;
      file->nWordsU = 0;
      file->nWordsY = 0;
      file->is_finished = false; 

      /* open file */
      char *file_name = filenames[i];
      FILE *f = fopen(file_name, "rb");

      if (f == NULL) {
        printf("[error] could not open the file %s\n",file_name);
        return EXIT_FAILURE;
      }

      /* while file is processing */
      // while ( !file->is_finished ) {

      /* Send a chunk of data to each worker process for processing */
      for (int worker = 1; worker < size; worker++) {

        // structure that has file's chunk to process and the results of that processing 
        struct ChunkData *chunk_data = (struct ChunkData *)malloc(sizeof(struct ChunkData));
        chunk_data->chunk = (int *)malloc(maxBytesPerChunk * sizeof(int));
        chunk_data->nWords = 0; chunk_data->nWordsA = 0; chunk_data->nWordsE = 0;chunk_data->nWordsI = 0; chunk_data->nWordsO = 0;chunk_data->nWordsU = 0; chunk_data->nWordsY = 0;
        chunk_data->is_finished = false;

        if (file->is_finished) {
          fclose(f); /* close the file pointer */
          break;
        }

        printf("[rank %d] getting chunk data for worker %d\n", rank, worker);
        get_chunk(chunk_data, f);

        printf("[rank %d] sending chunk data to rank %d\n", rank, worker);
        MPI_Isend ((char *) chunk_data, sizeof(struct ChunkData), MPI_BYTE, worker, 0, MPI_COMM_WORLD, &request); // MPI_ISend with request
      }

      // }


    }
  
  } else {
    struct ChunkData *chunk_data = (struct ChunkData *)malloc(sizeof(struct ChunkData));

    printf("[rank %d] waiting for work...\n", rank);
    MPI_Recv ((char *) chunk_data, sizeof (struct ChunkData), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("[rank %d] received chunk data from dispatcher!\n", rank);

    printf("chunk = ");
    for(int i = 0; i < 10; i++) {
        printf("%02x ", chunk_data->chunk[i]);
    }
    printf("\n");

    // count_words(chunk_data);

    printf("[rank %d] nWords = %d, nWordsA = %d, nWordsE = %d, nWordsI = %d, nWordsO = %d, nWordsU = %d, nWordsY = %d\n", rank, chunk_data->nWords, chunk_data->nWordsA, chunk_data->nWordsE, chunk_data->nWordsI, chunk_data->nWordsO, chunk_data->nWordsU, chunk_data->nWordsY);

  }

  MPI_Finalize ();

  // start counting the execution time
  (void) get_delta_time ();


  float exec_time = get_delta_time();
  printf("Execution time = %.6fs\n", exec_time);

  return EXIT_SUCCESS;
}



void get_chunk(struct ChunkData *data, FILE *file) {
  if (!all_work_done) {

    get_valid_chunk(data, file);

    /* 
    
    if (data->is_finished) {
      // avançar para o próximo ficheiro
      file_index++;
      fclose(file); // close the file pointer
      
      // ou dizer ao próximo worker que já não há benfica trabalhar
      if (numFiles == file_index) {
        all_work_done = true;
      }
    } 
    
    */
  }
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


void print_array(int arr[]) {
    for (int i = 0; i < 4000; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}