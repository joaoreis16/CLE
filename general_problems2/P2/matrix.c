#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int size;

void print_matrix(int matrix[][size], int rows, int columns);

int main(int argc, char** argv) {
    srand(time(0));

    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    int matrix_1[size][size];
    int matrix_2[size][size];
    int* sendData;

    if (rank == 0) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                matrix_1[i][j] = rand() % 100; // Generates a random number between 0 and 99
            }
        }

        printf("Matrix 1\n");
        print_matrix(matrix_1, size, size);

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                matrix_2[i][j] = rand() % 100; // Generates a random number between 0 and 99
            }
        }

        printf("\nMatrix 2\n");
        print_matrix(matrix_2, size, size);
        printf("\n");

        // ideia: enviar uma lista de listas: a primeira é uma linha da matriz 1, as outras são todas as colunas da matriz 2
        sendData = malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) sendData[i] = i;
    }

    // Broadcast the matrices to all processes
    MPI_Bcast(matrix_1, size * size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(matrix_2, size * size, MPI_INT, 0, MPI_COMM_WORLD);

    // split the sequence
    int recvData;
    MPI_Scatter(sendData, 1, MPI_INT, &recvData, 1, MPI_INT, 0, MPI_COMM_WORLD);

    printf("[%d]: row = %d\n", rank, recvData);

    for (int i = 0; i < size; i++) {
        int sum = 0;
        for (int j = 0; j < size; j++) {
            sum += matrix_1[recvData][j] * matrix_2[j][i];
            printf("%d * %d = %d\n", matrix_1[recvData][j],  matrix_2[j][i], matrix_1[recvData][j] * matrix_2[j][i]);
        }
        printf("rank [%d]\n", rank);
    }

    // enviar uma lista com os resultados dos calculos


    if (rank == 0) {
        // rank 0 recebe o resultado dos calculos e controi a matriz final
    }

    MPI_Finalize();
    return 0;
}

void print_matrix(int matrix[][size], int rows, int columns) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
}