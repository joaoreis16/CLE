/**
 *  \file sharedRegion.h (interface file)
 *
 *  \brief Shared Region for the text processing.
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


#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/**
 *  \brief Structure with the filename and file pointer to process.
 *
 *   It also stores the final results of the file processing.
 */
struct File {
  char *file_name;
  FILE *file;
  int nWords;
  int nWordsA;
  int nWordsE;
  int nWordsI;
  int nWordsO;
  int nWordsU;
  int nWordsY;
};

/**
 *  \brief Structure with the chunk data for processing.
 *
 *   It contains the chunk results of the file processing.
 */
struct ChunkData {
  int index;
  bool is_finished;
  int last_char;
  unsigned char *chunk;
  int chunk_size;
  int nWords;
  int nWordsA;
  int nWordsE;
  int nWordsI;
  int nWordsO;
  int nWordsU;
  int nWordsY;
};

/**
 *  \brief Initialization of the data transfer region.
 *
 *  Allocates the memory for an array of structures with the files passed
 *  as argument and initializes it with their names.
 *
 *  \param filenames contains the names of the files to be stored
 */
extern void initialize(char *filenames[]);

/**
 *  \brief Store the results of text processing in the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param workerId worker identification
 *  \param data structure with the results to be stored
 */
extern void saveChunkResults(unsigned int workerId, struct ChunkData *data);

/**
 *  \brief Get data to process from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param workerId worker identification
 *
 *  \param data structure that will store the chunk of chars to process
 */
extern void get_chunk(unsigned int workerId, struct ChunkData *data);

/**
 *  \brief Print results of the text processing.
 *
 *  Operation carried out by the main thread.
 */
extern void printResults();

#endif /* MONITOR_H */