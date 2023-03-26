
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#include "shared.h"


/** \brief consumer threads return status array */
int distributor_status;

/** \brief worker threads return status array */
int *workers_status;

/** \brief queue containing the workers' ids by order of arrival (time they made a request) */
int *waiting_work_queue;

/** \brief  */
int *work_assignment;

/** \brief number of worker threads */
int n_workers;

/** \brief distributor life cycle routine */
static void *distribute (void *id);

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
  n_workers = 4;            // number of worker threads
  char *filename = argv[1];     // binary file 
  int opt;                      // selected option

  do {
    switch ((opt = getopt(argc, argv, "hn:"))) {

      case 'n': // n. of workers
        if (atoi(optarg) < 1) {
          fprintf(stderr, "%s: number of worker threads must be greater or equal than 1\n", argv[0]);
          printUsage(argv[0]);
          return EXIT_FAILURE;
        }
        n_workers = (int)atoi(optarg);
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

  // storing file names in the shared region
  initialize(filename);

  workers_status     = malloc(sizeof(int) * n_workers);
  waiting_work_queue = malloc(sizeof(int) * n_workers);
  work_assignment    = malloc(sizeof(int) * n_workers);
  for (int i = 0; i < n_workers; i++) waiting_work_queue[i] = -1;
  for (int i = 0; i < n_workers; i++) work_assignment[i] = -1;
  
  printf("init work_assignment queue: ");
  print(work_assignment, n_workers);
  
  pthread_t *pthread_workers;             // workers' threads array
  pthread_t *pthread_distributor;         // distributor threads array
  unsigned int *workers;                  // workers application defined thread id array 
  unsigned int *distributor;              // distributor application defined thread id array 
  int *pStatus;                           // pointer to execution status 

  if (((pthread_workers = malloc (n_workers * sizeof (pthread_t))) == NULL) || 
      ((pthread_distributor = malloc (sizeof (pthread_t))) == NULL)         || 
      ((workers = malloc (n_workers * sizeof (int))) == NULL)               || 
      ((distributor = malloc (sizeof (int))) == NULL)) { 
    fprintf (stderr, "[error] on allocating space to both internal / external producer / consumer id arrays\n");
    return EXIT_FAILURE;
  }

  // creating distritutor
  int distributor_id = 0;
  if (pthread_create(&pthread_distributor[distributor_id], NULL, distribute,  &distributor[distributor_id]) != 0) {
    perror("[error] on creating thread distributor");
    return EXIT_FAILURE;
  }

  // creating worker threads 
  for (int i = 0; i < n_workers; i++) {
    workers[i] = i;   // add new worker with ID i

    if (pthread_create(&pthread_workers[i], NULL, worker, &workers[i]) != 0) {
      perror("[error] on creating thread worker");
      return EXIT_FAILURE;
    }
  }

  // waiting for the termination of the distributor thread
  if (pthread_join(pthread_distributor[distributor_id], (void *)&pStatus) != 0) {
    perror("[error] on waiting for distributor thread");
    return EXIT_FAILURE;
  }

  // waiting for the termination of the worker threads
  for (int i = 0; i < n_workers; i++) {
    if (pthread_join(pthread_workers[i], (void *)&pStatus) != 0) {
      perror("[error] on waiting for worker thread");
      return EXIT_FAILURE;
    }
  } 

  float exec_time = get_delta_time();
  printf("Execution time = %.6fs\n", exec_time);

  return EXIT_SUCCESS;
}


/**
 *  \brief Function worker.
 *
 *  Role of the worker threads
 *      • while there is work to be carried out
 *          − to request a sub-sequence of the sequence of integers
 *          − to sort it
 *          − to let the distributor thread know the work is done.
 *
 *  \param worker_id pointer to application defined worker identification
**/
static void *worker (void *worker_id) {
  unsigned int id = *((unsigned int *)worker_id); // worker id
  printf(">> Starting worker %d thread\n", id);
  bool requested = false;

  while(true) {

    // usleep((unsigned int) floor (10000.0 * random () / RAND_MAX + 1.5));

    // print(work_assignment, n_workers);

    if (work_assignment[id] == -1) {
      // send a request to distributor and wait for a work assignment
      if (!requested) {
        request_work(id);
        requested = true;
      }

    } else {
      // get work and sort integers
      sort_sequence(id);

      // send notification to distributor that the work that has been assigned is completed
      notify(id);

      requested = false;
      break;
    }

  }   

  workers_status[id] = EXIT_SUCCESS;
  pthread_exit(&workers_status[id]);
} 


/**
 *  \brief Function distributor.
 *
 *  Role of the distributor thread 
 *    • to read the sequence of integers from the binary file
 *    • to distribute sub-sequences of it to the worker threads.
 *
 *  \param distributor_id pointer to application defined distributor identification
 */
static void *distribute (void *distributor_id) {
  unsigned int id = *((unsigned int *)distributor_id); // worker id
  printf(">> Starting distributor thread\n");

  read_file();

  divide_work(n_workers);

  // esperar que um worker peça trabalho
  listen(id, n_workers);

  // quando todo o trabalho acabar, informar os workers


  distributor_status = EXIT_SUCCESS;
  pthread_exit(&distributor_status);
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
  fprintf (stderr, "\nSynopsis: %s filename [OPTIONS]\n"
           "  OPTIONS:\n"
           "  -n nWorkers    --- set the number of workers (default: 4)\n"
           "  -h             --- print this help\n", cmdName);
}