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

# define N 8


/* allusion to internal functions */

static double get_delta_time(void);

__global__ void mergeSubsequences(int* matrix_gpu, int subsequenceSize);

__global__ void sortSubsequence(int* matrix_gpu, int iter);

__device__ void swap(int* arr, int i, int j);

void validate(int *matrix);

void print_array(int arr[], int size, int file_size);
__global__ void print_device_array(int arr[], int size, int file_size);
bool contains(int *array, int size, int value);


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

    // create memory areas in host and device memory where the disk sectors data and sector numbers will be stored
    
    // alocar memória para a sequencia na memoria do cpu
    // reversar espaço na memoria do gpu
    
    // host allocation memory
    int *matrix = (int *)malloc(N * N * sizeof(int));
    
    // device allocation memory
    int *device_matrix;
    CHECK (cudaMalloc((void **) &device_matrix, sizeof(int) * N * N ));


    // initialize the host data
  
    // leitura do ficheiro
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
    /* int num;
    int i = 0;
    while(fread(&num, sizeof(int), 1, fp) == 1) {
        matrix[i] = num;
        i++;
    }*/

    int count = 0;

    file_size = 64;
    // Generate unique random numbers
    while (count < file_size) {
        int randomNum = rand() % 100; // Generate a random number between 0 and 1000

        // Check if the random number is already in the array
        if (!contains(matrix, count, randomNum)) {
            matrix[count] = randomNum;
            count++;
        }
    }

    fclose(fp); 

    print_array(matrix, N*N, N*N);

    // copy the host data to the device memory
    (void) get_delta_time ();
    CHECK (cudaMemcpy (device_matrix, matrix, N * N * sizeof(int), cudaMemcpyHostToDevice));
    printf ("dados copiados do cpu para o gpu\n");
    
    // Sorting iterations
    int numSubsequences = N;

    dim3 grid(numSubsequences, 1);
    dim3 block(1, 1);


    for (int iteration = 0; iteration < 3; iteration++) {

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
        mergeSubsequences<<<grid, block>>>(device_matrix, iteration); // sizeof(int) * mergeSize is the specification of resulting merge array size
        
        // printf("device array = ");
        print_device_array<<<1, 1>>>(device_matrix, N*N, N*N);

        // Wait for the merging kernel to finish
        cudaDeviceSynchronize();
    }	


    int * sorted_matrix = (int *)malloc( sizeof(int) * N * N );
    CHECK (cudaMemcpy (sorted_matrix, device_matrix, sizeof(int) * N * N, cudaMemcpyDeviceToHost));

    print_array(sorted_matrix, file_size, file_size);
    validate(sorted_matrix);

    // free device global memory 
    CHECK (cudaFree (device_matrix));

    // reset the device
    CHECK (cudaDeviceReset ());

    // free host memory subseqmergeSize
    free(matrix);
    free(sorted_matrix);

    return 0;
}


__global__ void mergeSubsequences(int* matrix_gpu, int iter) {
    int x = threadIdx.x + blockDim.x * blockIdx.x;
    int y = threadIdx.y + blockDim.y * blockIdx.y;
    int idx = blockDim.x * gridDim.x * y + x;  // Unique ID for each thread

    int subsequenceSize = (1 << iter) * N;  // Size of each subsequence
    int mergeSize = subsequenceSize * 2;  // Size of merged subsequences
    
    int* temp = new int[mergeSize];

    // Merge operation
    int i = 0, j = 0, k = 0;
    while (i < subsequenceSize && j < subsequenceSize) {

        int subseq1_idx = 2 * ((1 << iter) * idx) + (N * (i % N)) + (i / N);
        int subseq2_idx = 2 * ((1 << iter) * idx) + 1 + (N * (j % N)) + (j / N);

        // if(iter == 1) printf("subseq1_idx = %d | subseq2_idx = %d\n", subseq1_idx, subseq2_idx);

        // printf("thread(%d) : matrix_gpu[%d] = %d  | matrix_gpu[%d] = %d  \n", idx, subseq1_idx, matrix_gpu[subseq1_idx], subseq2_idx, matrix_gpu[subseq2_idx]);

        if (matrix_gpu[subseq1_idx] <= matrix_gpu[subseq2_idx]) {
            temp[k++] = matrix_gpu[subseq1_idx];
            i++; 
        } else {
            temp[k++] = matrix_gpu[subseq2_idx];
            j++;
        }
    }

    while (i < subsequenceSize) {
        int subseq1_idx = 2 * ((1 << iter) * idx) + (N * (i % N)) + (i / N);
        //printf("[1thread %d](%d) - %d\n", idx, subseq1_idx, matrix_gpu[subseq1_idx]);
        temp[k++] = matrix_gpu[subseq1_idx];
        i++;
    }

    while (j < subsequenceSize) {
        int subseq2_idx =  2 * ((1 << iter) * idx) + 1 + (N * (j % N)) + (j / N);
        // printf("[2thread %d](%d) - %d\n", idx, subseq2_idx, matrix_gpu[subseq2_idx]);
        temp[k++] = matrix_gpu[subseq2_idx];
        j++;
    }

    __syncthreads();

    //int numCols = mergeSize / N;

    /* for (i = 0; i < mergeSize; i++) {
        int col = i / stepSize;
        int subseq_idx = ((1 << iter) * idx) + (N * col);
        subseq_idx = ((1 << iter) * idx) + (N * col);
        if (idx == 0) printf("subseq_idx = %d | temp[i] = %d\n", subseq_idx, temp[i]);
        matrix_gpu[subseq_idx] = temp[i];
    } */

    /* for (i = 0; i < mergeSize; i++) {
        int subseq_idx = (idx * mergeSize) + i * N;
        if (idx == 0) printf("subseq_idx = %d | temp[i] = %d\n", subseq_idx, temp[i]);
        matrix_gpu[subseq_idx] = temp[i];
    }
 */

    for (i = 0; i < mergeSize; i++) {
        int subseq_idx = i * (1 << (2 - iter)) + idx;
        if (idx == 0) printf("subseq_idx = %d | temp[i] = %d\n", subseq_idx, temp[i]);
        matrix_gpu[subseq_idx] = temp[i];
    }

    if (idx == 0) {
        printf("thread %d = ", idx);
        for (int i = 0; i < mergeSize; i++) printf("%d ", temp[i]);
        printf("\n");
    }

    delete[] temp; 
}



__global__ void sortSubsequence(int* matrix_gpu, int iter) {
    int x = threadIdx.x + blockDim.x * blockIdx.x;
    int y = threadIdx.y + blockDim.y * blockIdx.y;
    int idx = blockDim.x * gridDim.x * y + x;  // Unique ID for each thread

    // Perform sorting (example: bubble sort)
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {

            int subseq_idx = ((1 << iter) * idx) + (N * (i % N)) + (i / N);
            int next_subseq_idx = ((1 << iter) * idx) + (N * (j % N)) + (j / N);

            // printf("subseq_idx = %d | next_subseq_idx = %d\n", subseq_idx, next_subseq_idx);

            if (matrix_gpu[subseq_idx] > matrix_gpu[next_subseq_idx]) {
                swap(matrix_gpu, subseq_idx, next_subseq_idx);
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