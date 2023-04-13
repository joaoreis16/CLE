#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char ** argv) {
    int rank;
    int size;
    char data[] = "I am here", *recData;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Request request = MPI_REQUEST_NULL;

    char msg[50];

    int prev_rank = rank - 1;
    int next_rank = rank + 1;

    if (rank == 0)        prev_rank = size - 1;
    if (rank == size - 1) next_rank = 0;
    

    if (rank == 0) { // first process
        sprintf(msg, "%s %d!", data, rank);
        printf ("[%d] Transmitted message: %s\n", rank, msg);
        MPI_Isend (msg, strlen (msg), MPI_CHAR, next_rank, 0, MPI_COMM_WORLD, &request);

        recData = malloc (100);
        MPI_Recv (recData, 100, MPI_CHAR, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf ("[%d] Received message:    %s \n", rank, recData);

    } else {
        recData = malloc (100);
        MPI_Recv (recData, 100, MPI_CHAR, prev_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf ("[%d] Received message:    %s \n", rank, recData);

        sprintf(msg, "%s %d!", data, rank);
        printf ("[%d] Transmitted message: %s\n", rank, msg);
        MPI_Send (msg, strlen (msg), MPI_CHAR, next_rank, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize ();
    return 0;
}