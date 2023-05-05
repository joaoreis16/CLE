#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <string.h>
#include <mpi.h>

#include "shared.h"


/** \brief storage region */
struct File *file;

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief print command usage */
static void printUsage (char *cmdName);

void printArray(int arr[], int size);

int main(int argc, char *argv[]) {

  // process command line arguments and set up variables
  int rank, size;
  int dispatcher = 0;

  // MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

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

    // start counting the execution time
    (void) get_delta_time ();

    int n_workers = size - 1;
    char *filename = argv[1];     // binary file 

    file = (struct File*)malloc(sizeof(struct File));
    file->filename = filename;
    file->file = NULL;

    read_file(file);
    printf("[rank %d] file readed\n", rank);

    divide_work(file, n_workers);
    printf("[rank %d] work divided\n", rank);


    for (int worker = 1; worker < size; worker++) {

      printf("[rank %d] sending task to worker %d\n", rank, worker);
      MPI_Send(&file->subsequences_length[ worker - 1 ], 1, MPI_INT, worker, 0, MPI_COMM_WORLD);    // send the subsequence size
      MPI_Send(file->subsequences[ worker - 1 ], file->subsequences_length[ worker - 1 ], MPI_INT, worker, 0, MPI_COMM_WORLD);  // then send the subsequence
    }

    for (int worker = 1; worker < size; worker++) { 
      MPI_Recv (file->subsequences[ worker - 1 ], file->subsequences_length[ worker - 1 ], MPI_INT, worker, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("[rank %d] received sorted subsequence from worker %d\n", rank, worker);
    }

    // merge all sorted subsequences
    while (file->all_subsequences_size != 1) {

      int worker = 1;

      for (int i = 0; i < file->all_subsequences_size - 1; i += 2) {
        int *subsequence1 = file->subsequences[i];
        int subsequence1_length = file->subsequences_length[i];

        int *subsequence2 = file->subsequences[i + 1];
        int subsequence2_length = file->subsequences_length[i + 1];

        // enviar uma mensagem ao worker a dizer que vai ter que fazer merge
        int merge_task = 1;
        MPI_Send(&merge_task, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);

        printf("[rank %d] send subsequences to merge to worker %d!\n", rank, worker);
        MPI_Send(&subsequence1_length, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);    // send the subsequence1 size
        MPI_Send(subsequence1, subsequence1_length, MPI_INT, worker, 0, MPI_COMM_WORLD);  // then send the subsequence1

        MPI_Send(&subsequence2_length, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);    // send the subsequence2 size
        MPI_Send(subsequence2, subsequence2_length, MPI_INT, worker, 0, MPI_COMM_WORLD);  // then send the subsequence2

        worker++;
      }

      printf("[rank %d] number of active workers = %d\n", rank, worker);

      // enviar uma mensagem aos workers restantes a dizer que vão estiar 
      for (int i = worker; i < size; i++) {
        int merge_task = 0;
        MPI_Send(&merge_task, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      }

      int new_size = worker - 1;
      if (file->all_subsequences_size % 2 != 0) {
        new_size = worker;
      }

      int *new_subsequences_length = (int *)malloc(new_size * sizeof(int));
      int **new_subsequences = (int **)malloc(new_size * sizeof(int *));
      int new_subsequences_size = 0;

      for (int i = 1; i < worker; i++) {
        int merged_sequence_size;
        MPI_Recv(&merged_sequence_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *merged_subsequence = (int*)malloc(sizeof(int) * merged_sequence_size);
        MPI_Recv (merged_subsequence, merged_sequence_size, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[rank %d] received merged subsequence from worker %d\n", rank, i);

        new_subsequences[i - 1] = merged_subsequence;
        new_subsequences_length[i - 1] = merged_sequence_size;
        new_subsequences_size++;
      }

      if (file->all_subsequences_size % 2 != 0) {
        new_subsequences[new_subsequences_size] = file->subsequences[file->all_subsequences_size - 1];
        new_subsequences_length[new_subsequences_size] = file->subsequences_length[file->all_subsequences_size - 1];
        new_subsequences_size++;
      }

      free(file->subsequences);
      free(file->subsequences_length);

      file->all_subsequences_size = new_subsequences_size;
      file->subsequences = new_subsequences;
      file->subsequences_length = new_subsequences_length;
    }

    // enviar mensagem que todo o trabalho acabou
    for (int worker = 1; worker < size; worker++) {
      int merge_task = -1;
      MPI_Send(&merge_task, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
    }

    file->sequence = file->subsequences[0];

    printf("[rank %d] ", rank);
    validate(file);

    float exec_time = get_delta_time();
    printf("Execution time = %.6fs\n", exec_time);

  } else {

    int subsequence_length;
    MPI_Recv(&subsequence_length, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int *subsequence = (int *)malloc(subsequence_length * sizeof(int));
    MPI_Recv(subsequence, subsequence_length, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("[rank %d] received subsequence from dispatcher!\n", rank);

    subsequence = sort_sequence(subsequence, subsequence_length);

    MPI_Send(subsequence, subsequence_length, MPI_INT, 0, 0, MPI_COMM_WORLD);
    printf("[rank %d] send sorted subsequence to dispatcher!\n", rank);

    // merge task

    // caso receba uma mensagem a dizer que tenha que fazer merge
    int merge_task = 0;
    while (true) {
      MPI_Recv(&merge_task, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if (merge_task == -1) break;

      if (merge_task) {

        int subsequence1_size;
        MPI_Recv(&subsequence1_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *subsequence1 = (int *)malloc(subsequence1_size * sizeof(int));
        MPI_Recv(subsequence1, subsequence1_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int subsequence2_size;
        MPI_Recv(&subsequence2_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *subsequence2 = (int *)malloc(subsequence2_size * sizeof(int));
        MPI_Recv(subsequence2, subsequence2_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("[rank %d] received subsequences to merge from dispatcher!\n", rank);

        int merged_sequence_size = (subsequence1_size + subsequence2_size);

        int *merged_subsequence = (int *)malloc(merged_sequence_size  * sizeof(int));
        merged_subsequence = merge_sequences(subsequence1, subsequence1_size, subsequence2, subsequence2_size);

        MPI_Send(&merged_sequence_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);    // send the subsequence1 size
        MPI_Send(merged_subsequence, merged_sequence_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("[rank %d] send merged subsequence to dispatcher!\n", rank);
      }
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
  fprintf (stderr, "\nSynopsis: %s filename \n", cmdName);
}


void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}
