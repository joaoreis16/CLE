#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE 16  // Size of the random generated sequence

void print_numbers(int *numbers, int size);


int main(int argc, char** argv) {
    int rank, size;
    int* random_numbers;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if(rank == 0) {
        srandom(getpid());
        random_numbers = malloc(ARRAY_SIZE * sizeof(int));

        for(int i = 0; i < ARRAY_SIZE; i++)  random_numbers[i] = ((double) rand() / RAND_MAX) * 1000;

        printf("random numbers: ");
        print_numbers(random_numbers, ARRAY_SIZE);
    }

    // split the sequence
    int* recvData;
    recvData = malloc(4 * sizeof(int));
    MPI_Scatter(random_numbers, 4, MPI_INT, recvData, 4, MPI_INT, 0, MPI_COMM_WORLD);

    int local_min, local_max;
    local_min = local_max = recvData[0];

    for (int i = 1; i < ARRAY_SIZE / size; i++) {
        if (recvData[i] < local_min) local_min = recvData[i];
        if (recvData[i] > local_max) local_max = recvData[i];
    }

    int global_min, global_max;
    MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if(rank == 0) printf("-> global min: %d\n-> global Max: %d\n", global_min, global_max);

    MPI_Finalize();
    return 0;
}

void print_numbers(int *numbers, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");
} 