/**
 *   Artur Romão e João Reis, May 2023
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include <cuda_runtime.h>

/**
 *   program configuration
 */

# define N 1024


/* allusion to internal functions */

static double get_delta_time(void);

__global__ void mergeSubsequences(int* matrix_gpu, int subsequenceSize);

__global__ void sortSubsequence(int* matrix_gpu, int iter);

__device__ void swap(int* arr, int i, int j);

void validate(int *matrix);

void print_array(int arr[], int size, int file_size);

__global__ void print_device_array(int arr[], int size, int file_size);

bool contains(int *array, int size, int value);

void sort(int* matrix, int iter, int numSubsequences, int idx);

void swap_cpu(int* arr, int i, int j);

void merge(int* matrix, int iter, int numSubsequences, int idx);


int main (int argc, char **argv)  {

    printf("%s Starting...\n", argv[0]);
    if (sizeof (unsigned int) != (size_t) 4)
        return 1;                                             // it fails with prejudice if an integer does not have 4 bytes

    // set up the device
    int dev = 0;

    cudaDeviceProp deviceProp;
    CHECK (cudaGetDeviceProperties (&deviceProp, dev));
    printf("Using Device %d: %s\n", dev, deviceProp.name);
    CHECK (cudaSetDevice (dev));

    // start counting the execution time
    (void) get_delta_time ();

    // create memory areas in host and device memory where the disk sectors data and sector numbers will be stored
    
    // host allocation memory
    int *matrix = (int *)malloc(N * N * sizeof(int));
    
    // device allocation memory
    int *device_matrix;
    CHECK (cudaMalloc((void **) &device_matrix, sizeof(int) * N * N ));


    // initialize the host data
  
    // read the file
    const char * filename = "datSeq1M.bin";

    FILE *fp = fopen(filename, "r");
    
    if (fp == NULL) {
      printf("Error: could not open file %s\n", filename);
      return EXIT_FAILURE;
    }

    printf("ficheiro lido com sucesso!\n");

    // Read the header of the binary file
    int file_size;
    if (fread(&file_size, sizeof(int), 1, fp) != 1) {
        printf("Error reading the file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    printf("file_size =  %d\n", file_size);

    // Read the contents of the file
    int num;
    int i = 0;
    while(fread(&num, sizeof(int), 1, fp) == 1) {
        matrix[i] = num;
        i++;
    }

    fclose(fp);

    // copy the host data to the device memory
    (void) get_delta_time ();
    CHECK (cudaMemcpy (device_matrix, matrix, N * N * sizeof(int), cudaMemcpyHostToDevice));
    printf ("copy the host data to the device memory\n");
    
    // Sorting iterations
    int numSubsequences = N;

    dim3 grid(numSubsequences, 1);
    dim3 block(1, 1);


    for (int iteration = 0; iteration < 10; iteration++) {

        printf("\n>> iteration %d\n", iteration);

        if (iteration == 0) {
            sortSubsequence<<<grid, block>>>(device_matrix, iteration);

            // Wait for the sorting kernel to finish
            cudaDeviceSynchronize();
        
            // print_device_array<<<1, 1>>>(device_matrix, N*N, N*N);
        } 

        numSubsequences /= 2;

        dim3 grid(numSubsequences, 1);
        dim3 block(1, 1);

        printf("starting merge\n");
        mergeSubsequences<<<grid, block>>>(device_matrix, iteration);

        // Wait for the merging kernel to finish
        cudaDeviceSynchronize();
    }	

    int * sorted_matrix = (int *)malloc( sizeof(int) * N * N );
    CHECK (cudaMemcpy (sorted_matrix, device_matrix, sizeof(int) * N * N, cudaMemcpyDeviceToHost));

    validate(sorted_matrix);

    float exec_time = get_delta_time();
    printf("GPU execution time = %.6fs\n", exec_time);

    // free device global memory 
    CHECK (cudaFree (device_matrix));

    // reset the device
    CHECK (cudaDeviceReset ());

    // free host memory subseqmergeSize
    free(sorted_matrix);


    (void) get_delta_time ();
    numSubsequences = N;

    // CPU
    for (int iteration = 0; iteration < 10; iteration++) {

        printf("\n>> iteration %d\n", iteration);

        if (iteration == 0) {
            for (int idx = 0; idx < numSubsequences; ++idx) {
                sort(matrix, iteration, numSubsequences, idx);
            }
        } 

        numSubsequences /= 2;
        
        for (int idx = 0; idx < numSubsequences; ++idx) {
            merge(matrix, iteration, numSubsequences, idx);
        }
    }	

    validate(matrix);

    float exec_time_cpu = get_delta_time();
    printf("CPU execution time = %.6fs\n", exec_time_cpu);

    free(matrix);

    return 0;
}


__global__ void mergeSubsequences(int* matrix_gpu, int iter) {
    int x = threadIdx.x + blockDim.x * blockIdx.x;
    int y = threadIdx.y + blockDim.y * blockIdx.y;
    int idx = blockDim.x * gridDim.x * y + x;  // Unique ID for each thread
    int subseq = 2 * N * (1 << iter) * idx;  // Starting index for each subsequence

    int subsequenceSize = (1 << iter) * N;  // Size of each subsequence
    int mergeSize = subsequenceSize * 2;  // Size of merged subsequences

    // Simple merge of two sorted subsequences
    int* subsequence1 = &matrix_gpu[subseq];
    int* subsequence2 = &matrix_gpu[subseq + subsequenceSize];
    
    int* temp = new int[mergeSize];

    // Merge operation
    int i = 0, j = 0, k = 0;
    while (i < subsequenceSize && j < subsequenceSize) {
        if (subsequence1[i] <= subsequence2[j]) {
            temp[k++] = subsequence1[i++];
        } else {
            temp[k++] = subsequence2[j++];
        }
    }
    // Copy the remaining elements of subsequence1[], if there are any
    while (i < subsequenceSize) {
        temp[k++] = subsequence1[i++];
    }
    // Copy the remaining elements of subsequence2[], if there are any
    while (j < subsequenceSize) {
        temp[k++] = subsequence2[j++];
    }

    __syncthreads();

    // Copy temp[] back to matrix_gpu
    for (i = 0; i < mergeSize; ++i) {
        matrix_gpu[subseq + i] = temp[i];
    }

    delete[] temp;  
}


__global__ void sortSubsequence(int* matrix_gpu, int iter) {
    int x = threadIdx.x + blockDim.x * blockIdx.x;
    int y = threadIdx.y + blockDim.y * blockIdx.y;
    int idx = blockDim.x * gridDim.x * y + x;  // Unique ID for each thread
    int subseq = N * (1 << iter) * idx;  // Starting index for each subsequence

    // Perform sorting (bubble sort)
    for (int i = subseq; i < subseq + N; i++) {
        for (int j = i + 1; j < subseq + N; j++) {

            if (matrix_gpu[i] > matrix_gpu[j]) {
                swap(matrix_gpu, i, j);
            }
        }
    }

    __syncthreads();    
}

__device__ void swap(int* arr, int i, int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}


static double get_delta_time(void)
{
  static struct timespec t0,t1;

  t0 = t1;
  if(clock_gettime(CLOCK_MONOTONIC,&t1) != 0)
  {
    perror("clock_gettime");
    exit(1);
  }
  return (double)(t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double)(t1.tv_nsec - t0.tv_nsec);
}


void validate(int *matrix) {

    int size = N * N;
    int i;
    for (i = 0; i < size; i++) {

        if (i == (size - 1))  {
            printf ("Everything is OK!\n");
            break;
        }

        if (matrix[i] > matrix[i+1]) { 
            printf ("Error in position %d between element %d and %d\n", i, matrix[i], matrix[i+1]);
            break;
        }
    }

    printf ("\n");
}


void print_array(int arr[], int size, int file_size) {
    for (int i = file_size - size; i < file_size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}


__global__ void print_device_array(int arr[], int size, int file_size) {
    for (int i = file_size - size; i < file_size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}


bool contains(int *array, int size, int value) {
    for (int i = 0; i < size; ++i) {
        if (array[i] == value) {
            return true;
        }
    }
    return false;
}



void sort(int* matrix, int iter, int numSubsequences, int idx) {
    int subseq = N * (1 << iter) * idx;  // Starting index for each subsequence

    // Perform sorting (example: bubble sort)
    for (int i = subseq; i < subseq + N; i++) {
        for (int j = i + 1; j < subseq + N; j++) {

            if (matrix[i] > matrix[j]) {
                swap_cpu(matrix, i, j);
            }
        }
    }
}

void merge(int* matrix, int iter, int numSubsequences, int idx) {
    int subseq = 2 * N * (1 << iter) * idx;  // Starting index for each subsequence

    int subsequenceSize = (1 << iter) * N;  // Size of each subsequence
    int mergeSize = subsequenceSize * 2;  // Size of merged subsequences

    // Simple merge of two sorted subsequences
    int* subsequence1 = &matrix[subseq];
    int* subsequence2 = &matrix[subseq + subsequenceSize];
    
    int* temp = new int[mergeSize];

    // Merge operation
    int i = 0, j = 0, k = 0;
    while (i < subsequenceSize && j < subsequenceSize) {
        if (subsequence1[i] <= subsequence2[j]) {
            temp[k++] = subsequence1[i++];
        } else {
            temp[k++] = subsequence2[j++];
        }
    }
    // Copy the remaining elements of subsequence1[], if there are any
    while (i < subsequenceSize) {
        temp[k++] = subsequence1[i++];
    }
    // Copy the remaining elements of subsequence2[], if there are any
    while (j < subsequenceSize) {
        temp[k++] = subsequence2[j++];
    }

    // Copy temp[] back to matrix
    for (i = 0; i < mergeSize; ++i) {
        matrix[subseq + i] = temp[i];
    }

    delete[] temp; 
}


void swap_cpu(int* arr, int i, int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

