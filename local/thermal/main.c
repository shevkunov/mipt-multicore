#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int myrank, size;
MPI_Status status;

double time_begin, time_end;

char fname[] = "data.py";
char time_fname[] = "time_on_00.txt";

double rho;
double c;
double lambda;
double k;
double h;
double tau;
double T;

double u0;

double ud;
double uu;
double ul;
double ur;

double Ly;
double Lx;



void init() {
    rho = 8960;
    c = 380;
    lambda = 401;
    k = lambda / (rho * c);
    h = 0.01;
    tau = 0.1;
    T = 60;

    u0 = 5;

    ud = 0;
    uu = 100;
    ul = 80;
    ur = 30;

    Ly = 0.5;
    Lx = 0.5;
}

double** malloc_array(int y_cnt, int x_cnt, double init_value) {
    double** a;
    a = malloc(sizeof(double*) * y_cnt);
    for (int i = 0; i < y_cnt; ++i) {
        a[i] = malloc(sizeof(double) * x_cnt);
        for (int j = 0; j < x_cnt; ++j) {
            a[i][j] = init_value;
        }
    }
    return a;
}

void free_array(double** a, int y_cnt) {
    for (int i = 0; i < y_cnt; ++i) {
        free(a[i]);
    }
    free(a);
}

void save_data(double** a, int y_cnt, int x_cnt) {
    FILE *f = fopen(fname, "wt");
    if (!f) {
        printf("Unsuccessful fopen. Terminated.\n");
        exit(1);
    }
    fprintf(f, "size = (%d, %d)\n", y_cnt, x_cnt);
    fprintf(f, "values = [\n");
    for (int i = 0; i < y_cnt; ++i) {
        fprintf(f, "[");
        for (int j = 0; j < x_cnt; ++j) {
            fprintf(f, "%f, ", a[i][j]);
        }
        fprintf(f, "], \n");
    }
    fprintf(f, "]\n");
    fclose(f);
}

int get_y_border(int y_cnt, int rank) {
    int addition = y_cnt % size;
    int res = y_cnt / size * rank;
    if (rank <= addition) {
        res += rank;
    } else {
        res += addition;
    }
    if (rank == 0) {
        res += 1;
    }
    if (rank == size) {
        res -= 1;
    }
    return res;
}

void multithread_solve() {
    int x_cnt = Lx / h + 1;  // a[0][0] - (0,0), a[50][50] - (0.5, 0.5)
    int y_cnt = Ly / h + 1;

    int my_y_l = get_y_border(y_cnt, myrank);
    int my_y_r = get_y_border(y_cnt, myrank + 1);
    int my_y_cnt = my_y_r - my_y_l + 2;
#ifdef LOG
    printf("Id (%d), my_y_l = %d, my_y_r = %d, my_y_cnt = %d\n",
           myrank, my_y_l, my_y_r, my_y_cnt);
    fflush(stdout);
#endif

    double koeff = k * tau / (h * h);
    double time_now = 0.;

    double** u;
    double** v;

    u = malloc_array(my_y_cnt, x_cnt, u0);
    v = malloc_array(my_y_cnt, x_cnt, u0);

    for (int i = 0; i < my_y_cnt; ++i) {
        v[i][0] = u[i][0] =  ud;
        v[i][x_cnt - 1] = u[i][x_cnt - 1] = uu;
    }

    for (int i = 0; i < x_cnt; ++i) {
        if (my_y_l == 1) {
            v[0][i] = u[0][i] =  ul;
        }
        if (my_y_r + 1 == y_cnt) {
            v[my_y_cnt - 1][i] = u[my_y_cnt - 1][i] = ur;
        }
    }

    while (time_now < T) {
#ifdef LOG
        printf("while %f, (%d)\n", time_now, myrank);
        fflush(stdout);
#endif
        for (int i = 1; i < my_y_cnt - 1; ++i) {
            for (int j = 1; j < x_cnt - 1; ++j) {
                v[i][j] = u[i][j] +
                        koeff * (u[i+1][j] - 2. * u[i][j] + u[i-1][j]);
            }
        }

        for (int i = 1; i < my_y_cnt - 1; ++i) {
            for (int j = 1; j < x_cnt - 1; ++j) {
                u[i][j] = v[i][j] +
                        koeff * (v[i][j+1] - 2. * v[i][j] + v[i][j-1]);
            }
        }

        if (myrank & 1) {
            // send - recieve
            if (myrank) {
                MPI_Send(u[1], x_cnt, MPI_DOUBLE,
                        myrank - 1, myrank, MPI_COMM_WORLD);
            }
            if (myrank + 1 != size) {
                MPI_Send(u[my_y_cnt - 2], x_cnt, MPI_DOUBLE,
                        myrank + 1, myrank, MPI_COMM_WORLD);
            }

            if (myrank) {
                MPI_Recv(u[0], x_cnt, MPI_DOUBLE, myrank - 1, myrank - 1,
                                    MPI_COMM_WORLD, &status);
            }
            if (myrank + 1 != size) {
                MPI_Recv(u[my_y_cnt - 1], x_cnt, MPI_DOUBLE,
                        myrank + 1, myrank + 1, MPI_COMM_WORLD, &status);
            }
        } else {
            // recieve - send
            if (myrank) {
                MPI_Recv(u[0], x_cnt, MPI_DOUBLE, myrank - 1, myrank - 1,
                                    MPI_COMM_WORLD, &status);
            }
            if (myrank + 1 != size) {
                MPI_Recv(u[my_y_cnt - 1], x_cnt, MPI_DOUBLE,
                        myrank + 1, myrank + 1, MPI_COMM_WORLD, &status);
            }

            if (myrank) {
                MPI_Send(u[1], x_cnt, MPI_DOUBLE,
                        myrank - 1, myrank, MPI_COMM_WORLD);
            }
            if (myrank + 1 != size) {
                MPI_Send(u[my_y_cnt - 2], x_cnt, MPI_DOUBLE,
                        myrank + 1, myrank, MPI_COMM_WORLD);
            }
        }
        time_now += tau;
    }
#ifdef LOG
    printf("Finalization...");
    fflush(stdout);
#endif
    if (myrank == 0) {
        double** u_final = malloc_array(y_cnt, x_cnt, -1e9);
        for (int i = 0; i < my_y_cnt; ++i) {
            for (int j = 0; j < x_cnt; ++j) {
                u_final[i][j] = u[i][j];
            }
        }

        for (int rk = 1; rk < size; ++rk) {
            int y_l = get_y_border(y_cnt, rk);
            int y_r = get_y_border(y_cnt, rk + 1);
            for (int i = y_l; i < y_r; ++i) {
#ifdef LOG
                printf("Master recv %d...", i);
                fflush(stdout);
#endif
                MPI_Recv(u_final[i], x_cnt, MPI_DOUBLE, rk, rk,
                                    MPI_COMM_WORLD, &status);
            }
        }

        for (int j = 0; j < x_cnt; ++j) {
            u_final[y_cnt - 1][j] = ur;
        }
#ifdef LOG
        printf("Master recved all");
        fflush(stdout);
#endif
        save_data(u_final, y_cnt, x_cnt);
    } else {
        int rightest = my_y_cnt - 1;
        if (myrank + 1 == size) {
            rightest += 1;
        }
        for (int i = 1; i < rightest; ++i) {
#ifdef LOG
            printf("Slave send %d...", i);
            fflush(stdout);
#endif
            MPI_Send(u[i], x_cnt, MPI_DOUBLE,
                    0, myrank, MPI_COMM_WORLD);
        }
#ifdef LOG
        printf("Slave sent all");
        fflush(stdout);
#endif
    }
}

void solo_solve() {
    int x_cnt = Lx / h + 1;  // a[0][0] - (0,0), a[50][50] - (0.5, 0.5)
    int y_cnt = Ly / h + 1;

    double koeff = k * tau / (h * h);
    double time_now = 0.;

    double** u;
    double** v;

    u = malloc_array(y_cnt, x_cnt, u0);
    v = malloc_array(y_cnt, x_cnt, -1e9);

    for (int i = 0; i < y_cnt; ++i) {
        v[i][0] = u[i][0] =  ud;
        v[i][x_cnt - 1] = u[i][x_cnt - 1] = uu;
    }

    for (int i = 0; i < x_cnt; ++i) {
        v[0][i] = u[0][i] =  ul;
        v[x_cnt - 1][i] = u[x_cnt - 1][i] = ur;
    }

    while (time_now < T) {
        for (int i = 1; i < y_cnt - 1; ++i) {
            for (int j = 1; j < x_cnt - 1; ++j) {
                v[i][j] = u[i][j] +
                        koeff * (u[i+1][j] - 2. * u[i][j] + u[i-1][j]);
            }
        }

        for (int i = 1; i < y_cnt - 1; ++i) {
            for (int j = 1; j < x_cnt - 1; ++j) {
                u[i][j] = v[i][j] +
                        koeff * (v[i][j+1] - 2. * v[i][j] + v[i][j-1]);
            }
        }

        time_now += tau;
    }

    save_data(u, y_cnt, x_cnt);
}

int main(int argc, char *argv[])
{
    // printf("Before finalize.\n");

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    printf("Process %d of %d ready.\n", myrank, size);

    MPI_Barrier(MPI_COMM_WORLD);
    time_begin = MPI_Wtime();

    init();

    if (size > 1) {
        multithread_solve();
    } else {
        solo_solve();
    }

    MPI_Barrier(MPI_COMM_WORLD);
    time_end = MPI_Wtime();

    if (myrank == 0) {
        time_fname[9] = size % 10 + '0';
        time_fname[8] = size /10 + '0';
        FILE* f = fopen(time_fname, "a");
        if (!f) {
            printf("Unsuccessful fopen. Terminated.\n");
            exit(1);
        }
        fprintf(f, "%.9f\n", time_end - time_begin);
        fclose(f);
    }

    MPI_Finalize();
    printf("After finalize. Process %d\n", myrank);
    return 0;
}
