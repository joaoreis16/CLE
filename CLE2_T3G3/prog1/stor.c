#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct XYZ {
    int *a;
    double b;
    char c;
};

int main(int argc, char *argv[]) {
    int rank, size;
    struct XYZ *sndData = (struct XYZ *) malloc(sizeof(struct XYZ));
    struct XYZ *recData = (struct XYZ *) malloc(sizeof(struct XYZ));

    sndData->a = (int *) malloc(4 * sizeof(int));
    sndData->a[0] = 1;
    sndData->a[1] = 2;
    sndData->a[2] = 3;
    sndData->a[3] = 4;
    sndData->b = 0.5;
    sndData->c = 'c';

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) printf("Too few processes!\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    if (rank == 0) {
        printf("Transmitted message: - %.3f - %c - lst = ", sndData->b, sndData->c);
        for (int i = 0; i < 4; i++) {
            printf("%d ", sndData->a[i]);
        }
        printf("\n");

        MPI_Send(sndData, sizeof(struct XYZ), MPI_BYTE, 1, 0, MPI_COMM_WORLD);

    } else if (rank == 1) {
        MPI_Recv(recData, sizeof(struct XYZ), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("Received message: %.3f - %c - lst = ", recData->b, recData->c);
        for (int i = 0; i < 4; i++) {
            printf("%d ", recData->a[i]);
        }
        printf("\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}