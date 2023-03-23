/**
 *  \file shared.c (interface file)
 *
 *  \brief Memory Shared Region for the text processing problem with multithreading.
 *
 *  Synchronization based on monitors.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  This shared region will use the array of structures initialized by
 *  the main thread.
 * 
 *  Workers can access the shared region to obtain data to process from that
 *  array of structures. They can also store the partial results of the
 *  processing done.
 * 
 *  There is also a function to print out the final results, which is
 *  used after there is no more data to be processed.
 * 
 *  Monitored Methods:
 *     \li get_chunk - operation carried out by worker threads to get a chunk of text (size = maxBytesPerChunk).
 *     \li update_counters - operation carried out by worker threads to update the word counters each time a chunk is processed.
 *
 *  Unmonitored Methods:
 *     \li initialize - operation carried out by the main thread to allocate memory and start counters.
 *     \li process_chunk - operation carried out by the main thread to process the text chunk.
 *     \li reset_struct - operation carried out by the main thread to reset the variables of the struct ChunkData.
 *     \li print_results - operation carried out by the main thread to print the final results.
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
  unsigned int *chunk;
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
 *  \brief Update counter variables of the struct File.
 *
 *  Operation carried out by the workers.
 *
 *  \param id worker identification
 *  \param data structure that will store the chunk of chars to process
 */
extern void update_counters(unsigned int id, struct ChunkData *data);

/**
 *  \brief Get data to process from the data transfer region.
 *
 *  Operation carried out by the workers.
 *
 *  \param id worker identification
 *
 *  \param data structure that will store the chunk of chars to process
 */
extern void get_chunk(unsigned int id, struct ChunkData *data);

/**
 *  \brief Process the chunk data.
 *
 *  Operation carried out by the main.
 *
 *  \param id worker identification
 *  \param data structure that will store the chunk of chars to process
 */
extern void process_chunk(unsigned int id, struct ChunkData *data);

/**
 *  \brief Reset the variables of the struct ChunkData.
 *
 *  Operation carried out by the main.
 *
 *  \param data structure that will store the chunk of chars to process
 */
extern void reset_struct(struct ChunkData *data);

/**
 *  \brief Print results of the text processing.
 *
 *  Operation carried out by the main thread.
 */
extern void print_results();

#endif /* MONITOR_H */