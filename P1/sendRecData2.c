#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char ** argv) {
    int rank;
    int size;
    char data[]  = "I am alive and well", *recData;
    char data2[] = "I am alive and well too";

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Request request = MPI_REQUEST_NULL;

    char msg[50];
    

    if (rank == 0) { // first process

        for (int i = 1; i < size; i++) {
            printf ("[0] Transmitted message to rank %d\n", i);
            MPI_Isend (data, strlen (data), MPI_CHAR, i, 0, MPI_COMM_WORLD, &request);
        }

        for (int i = 1; i < size; i++) {
            recData = malloc (100);
            MPI_Recv (recData, 100, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf ("[0] Received message from rank %d: %s \n", i, recData);
        }

    } else {
        recData = malloc (100);
        MPI_Recv (recData, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf ("[%d] Received message from rank 0\n", rank);

        sprintf(msg, "%s!",  data2);
        printf ("[%d] Transmitted message: %s\n", rank, msg);
        MPI_Send (msg, strlen (msg), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize ();
    return 0;
}