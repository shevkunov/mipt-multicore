#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
	int i;
	int array[10];
	int myrank, size;
	MPI_Status Status;


	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	
	printf("I am %d of %d\n", myrank, size);
	if (myrank == 0) {
		for (i = 0; i < 10; ++i) {
			array[i] = i;
		}
		MPI_Send(&array[5], 5, MPI_INT, 1, 1, MPI_COMM_WORLD);
		/* (from, how many, type, to whom, tag, ???) */
	}	
	if (myrank == 1) {
		/* (where to write, how many, type, from who, tag, communicator, status) */
		MPI_Recv(array, 5, MPI_INT, 0, 1, MPI_COMM_WORLD, &Status);
		for (i = 0; i < 5; ++i) {
			printf("%d ", array[i]);
		}
		printf("\n");
	}

	MPI_Finalize();

	return 0;
}
