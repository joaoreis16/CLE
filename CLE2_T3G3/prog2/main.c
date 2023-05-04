#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <string.h>

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

void print_results(struct File *file_data);

void reset_struct(struct ChunkData *data);

int main(int argc, char *argv[]) {

  // process command line arguments and set up variables
  int opt;                      // selected option

  int rank, size;
  int dispatcher = 0;

  // MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Request request = MPI_REQUEST_NULL;

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

    char *filename = argv[1];     // binary file 

    // start counting the execution time
    (void) get_delta_time ();

    

    float exec_time = get_delta_time();
    printf("Execution time = %.6fs\n", exec_time);

  } else {
    
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
  fprintf (stderr, "\nSynopsis: %s filename \n", cmdName);
}
