/**
 *  \file producersConsumers.c (implementation file)
 *
 *  \brief Problem name: Producers / Consumers.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author António Rui Borges - March 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#include "probConst.h"
#include "fifo.h"

/** \brief return status on monitor initialization */
int statusInitMon;

/** \brief producer threads return status array */
int *statusProd;

/** \brief consumer threads return status array */
int *statusCons;

/** \brief number of storage positions in the data transfer region */
int nStorePos = 4;

/** \brief producer life cycle routine */
static void *producer (void *id);

/** \brief consumer life cycle routine */
static void *consumer (void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief print command usage */
static void printUsage (char *cmdName);

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (producers and consumers) and
 *  waiting for their termination.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */

int main (int argc, char *argv[])
{
  /* process command line options */

  int opt;                                                                                        /* selected option */
  int nThreads = 4;                                                               /* number of threads to be created */

  do
  { switch ((opt = getopt (argc, argv, "t:p:h")))
    { case 't': /* number of threads to be created */
                if (atoi (optarg) <= 0)
                   { fprintf (stderr, "%s: non positive number\n", basename (argv[0]));
                     printUsage (basename (argv[0]));
                     return EXIT_FAILURE;
                   }
                nThreads = (int) atoi (optarg);
                if (nThreads > N)
                   { fprintf (stderr, "%s: too many threads\n", basename (argv[0]));
                     printUsage (basename (argv[0]));
                     return EXIT_FAILURE;
                   }
                break;
      case 'p': /* number of storage positions in the data transfer region */
                if (atoi (optarg) <= 0)
                   { fprintf (stderr, "%s: non positive number\n", basename (argv[0]));
                     printUsage (basename (argv[0]));
                     return EXIT_FAILURE;
                   }
                nStorePos = (int) atoi (optarg);
                if (nStorePos > K)
                   { fprintf (stderr, "%s: too many storage positions\n", basename (argv[0]));
                     printUsage (basename (argv[0]));
                     return EXIT_FAILURE;
                   }
                break;
      case 'h': /* help mode */
                printUsage (basename (argv[0]));
                return EXIT_SUCCESS;
      case '?': /* invalid option */
                fprintf (stderr, "%s: invalidxx option\n", basename (argv[0]));
    	        printUsage (basename (argv[0]));
                return EXIT_FAILURE;
      case -1:  break;
    }
  } while (opt != -1);
  if (optind < argc)
     { fprintf (stderr, "%s: invalid format\n", basename (argv[0]));
       printUsage (basename (argv[0]));
       return EXIT_FAILURE;
     }
  if (((statusProd = malloc (nThreads * sizeof (int))) == NULL) ||
      ((statusCons = malloc (nThreads * sizeof (int))) == NULL))
     { fprintf (stderr, "error on allocating space to the return status arrays of producer / consumer threads\n");
       exit (EXIT_FAILURE);
     }

  pthread_t *tIdProd,                                                          /* producers internal thread id array */
            *tIdCons;                                                          /* consumers internal thread id array */
  unsigned int *prod,                                               /* producers application defined thread id array */
               *cons;                                               /* consumers application defined thread id array */
  int i;                                                                                        /* counting variable */
  int *pStatus;                                                                       /* pointer to execution status */

  /* initializing the application defined thread id arrays for the producers and the consumers and the random number
     generator */

  if (((tIdProd = malloc (nThreads * sizeof (pthread_t))) == NULL) ||
      ((tIdCons = malloc (nThreads * sizeof (pthread_t))) == NULL) ||
      ((prod = malloc (nThreads * sizeof (unsigned int))) == NULL) ||
      ((cons = malloc (nThreads * sizeof (unsigned int))) == NULL))
     { fprintf (stderr, "error on allocating space to both internal / external producer / consumer id arrays\n");
       exit (EXIT_FAILURE);
     }
  for (i = 0; i < nThreads; i++)
    prod[i] = i;
  for (i = 0; i < nThreads; i++)
    cons[i] = i;
  srandom ((unsigned int) getpid ());
  (void) get_delta_time ();

  /* generation of intervening entities threads */

  for (i = 0; i < nThreads; i++)
    if (pthread_create (&tIdProd[i], NULL, producer, &prod[i]) != 0)                              /* thread producer */
       { perror ("error on creating thread producer");
         exit (EXIT_FAILURE);
       }
  for (i = 0; i < nThreads; i++)
    if (pthread_create (&tIdCons[i], NULL, consumer, &cons[i]) != 0)                             /* thread consumer */
       { perror ("error on creating thread consumer");
         exit (EXIT_FAILURE);
       }

  /* waiting for the termination of the intervening entities threads */

  printf ("\nFinal report\n");
  for (i = 0; i < nThreads; i++)
  { if (pthread_join (tIdProd[i], (void *) &pStatus) != 0)                                       /* thread producer */
       { perror ("error on waiting for thread producer");
         exit (EXIT_FAILURE);
       }
    printf ("thread producer, with id %u, has terminated: ", i);
    printf ("its status was %d\n", *pStatus);
  }
  for (i = 0; i < nThreads; i++)
  { if (pthread_join (tIdCons[i], (void *) &pStatus) != 0)                                       /* thread consumer */
       { perror ("error on waiting for thread customer");
         exit (EXIT_FAILURE);
       }
    printf ("thread consumer, with id %u, has terminated: ", i);
    printf ("its status was %d\n", *pStatus);
  }
  printf ("\nElapsed time = %.6f s\n", get_delta_time ());

  exit (EXIT_SUCCESS);
}

/**
 *  \brief Function producer.
 *
 *  Its role is to simulate the life cycle of a producer.
 *
 *  \param par pointer to application defined producer identification
 */

static void *producer (void *par)
{
  unsigned int id = *((unsigned int *) par),                                                          /* producer id */
               val;                                                                                /* produced value */
  int i;                                                                                        /* counting variable */

  for (i = 0; i < M; i++)
  { val = 1000 * id + i;                                                                          /* produce a value */
    putVal (id, val);                                                                               /* store a value */
    usleep((unsigned int) floor (10000.0 * random () / RAND_MAX + 1.5));                           /* do something else */
  }

  statusProd[id] = EXIT_SUCCESS;
  pthread_exit (&statusProd[id]);
}

/**
 *  \brief Function consumer.
 *
 *  Its role is to simulate the life cycle of a consumer.
 *
 *  \param par pointer to application defined consumer identification
 */

static void *consumer (void *par)
{
  unsigned int id = *((unsigned int *) par),                                                          /* consumer id */
               val;                                                                                /* produced value */
  int i;                                                                                        /* counting variable */

  for (i = 0; i < M; i++)
  { usleep((unsigned int) floor (10000.0 * random () / RAND_MAX + 1.5));                           /* do something else */
    val = getVal (id);                                                                           /* retrieve a value */
    printf ("The value %u was produced by the thread P%u and consumed by the thread C%u.\n",      /* consume a value */
            val % 1000, val / 1000, id);
  }

  statusCons[id] = EXIT_SUCCESS;
  pthread_exit (&statusCons[id]);
}

/**
 *  \brief Get the process time that has elapsed since last call of this time.
 *
 *  \return process elapsed time
 */

static double get_delta_time(void)
{
  static struct timespec t0, t1;

  t0 = t1;
  if(clock_gettime (CLOCK_MONOTONIC, &t1) != 0)
  {
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

static void printUsage (char *cmdName)
{
  fprintf (stderr, "\nSynopsis: %s [OPTIONS]\n"
           "  OPTIONS:\n"
           "  -t nThreads  --- set the number of threads to be created (default: 4)\n"
           "  -p nStorePos --- set the number of storage positions in the data transfer region (default: 4)\n"
           "  -h           --- print this help\n", cmdName);
}
