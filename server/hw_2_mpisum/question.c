#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main(int argc, char *argv[]) {
    int myrank, size;
    MPI_Status status;
    
    printf("Before finalize.\n");
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    
    printf("Process %d of %d ready.\n", myrank, size);
    
    MPI_Finalize();
    printf("After finalize. Process %d\n", myrank);
    return 0;
}
