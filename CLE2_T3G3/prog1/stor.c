#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct XYZ {
    int a[4];
    double b;
    char c;
};

int main(int argc, char *argv[]) {
    int rank, size;
    struct XYZ sndData;
    struct XYZ recData;

    sndData.a[0] = 1;
    sndData.a[1] = 2;
    sndData.a[2] = 3;
    sndData.a[3] = 4;
    sndData.b = 0.5;
    sndData.c = 'c';

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) printf("Too few processes!\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    MPI_Datatype XYZtype;
    int blocklengths[3] = {4, 1, 1};
    MPI_Aint displacements[3] = {offsetof(XYZ, a), offsetof(XYZ, b), offsetof(XYZ, c)};
    MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE, MPI_CHAR};

    MPI_Type_create_struct(3, blocklengths, displacements, types, &XYZtype);
    MPI_Type_commit(&XYZtype);

    if (rank == 0) {
        printf("Transmitted message: - %.3f - %c - lst = ", sndData.b, sndData.c);
        for (int i = 0; i < 4; i++) {
            printf("%d ", sndData.a[i]);
        }
        printf("\n");

        MPI_Send(&sndData, 1, XYZtype, 1, 0, MPI_COMM_WORLD);

    } else if (rank == 1) {
        MPI_Recv(&recData, 1, XYZtype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Received message: %.3f - %c - lst = ", recData.b, recData.c);
        for (int i = 0; i < 4; i++) {
            printf("%d ", recData.a[i]);
        }
        printf("\n");
    }

    MPI_Type_free(&XYZtype);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
