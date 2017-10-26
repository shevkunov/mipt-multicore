#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int myrank, size;
MPI_Status status;

const int N = 1000;

int get_problem_size(int rank) {
    return N / (size - 1) + (N % (size - 1) >= rank);
}

void master() {
    int a[N];
    int shift = 0;
   
    long long check_sum = 0;
    long long mpi_sum = 0;
    
    if (size <= 1) { // there are no slaves :(
        printf("Assuming NP > 1. Terminated.\n");
        return;
    }
    
    for (int i = 0; i < N; ++i) {
        a[i] = i;
        check_sum += a[i];
    }
   
    for (int i = 1; i < size; ++i) {
        int problem_size = get_problem_size(i); // not size!
        MPI_Send(a + shift, problem_size, MPI_INT, i, i, MPI_COMM_WORLD);
        shift += problem_size;       
    }
    
    printf("N = %d\n", N);
    printf("Total problems send = %d\n", shift);
    
    for (int i = 1; i < size; ++i) {
        long long mpi_return;
        MPI_Recv(&mpi_return, 1, MPI_LONG_LONG_INT, i, i,
                    MPI_COMM_WORLD, &status);
        mpi_sum += mpi_return;
        printf("Process %d returned sum = %lld\n", i, mpi_return);
    }
    
    printf("Sum for all processes = %lld\n", mpi_sum);
    printf("Master-process sum = %lld\n", check_sum);
    if (check_sum == mpi_sum) {
        printf("Sum equals: YES\n");
    } else {
        printf("Sum equals: NO\n");
    }
}

void slave() {
    int problem_size = get_problem_size(myrank);
    int a[problem_size];
    
    long long int result = 0;
    
    MPI_Recv(&a, problem_size, MPI_INT, 0, myrank, MPI_COMM_WORLD, &status);
    for (int i = 0; i < problem_size; ++i) {
        result += a[i];
    }
    MPI_Send(&result, 1, MPI_LONG_LONG_INT, 0, myrank, MPI_COMM_WORLD);
}

int main(int argc, char *argv[]) {

    
    // printf("Before finalize.\n");
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    
    printf("Process %d of %d ready.\n", myrank, size);
    
    if (myrank == 0) {
        master();
    } else {
        slave();
    }
    
    MPI_Finalize();
    // printf("After finalize. Process %d\n", myrank);
    return 0;
}
