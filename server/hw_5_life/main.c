#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

FILE* open_file(const char* fname, const char *__restrict how) {
    FILE *f = fopen(fname, how);
    if (!f) {
        printf("Unable to open file %s. Terminated.\n", fname);
        fflush(stdout);
        exit(-1);
    }
    return f;
}

char** malloc_array(int y_cnt, int x_cnt, char init_value) {
    char** a;
    a = malloc(sizeof(char*) * y_cnt);
    for (int i = 0; i < y_cnt; ++i) {
        a[i] = malloc(sizeof(char) * x_cnt);
        for (int j = 0; j < x_cnt; ++j) {
            a[i][j] = init_value;
        }
    }
    return a;
}

void free_array(char** a, int y_cnt) {
    for (int i = 0; i < y_cnt; ++i) {
        free(a[i]);
    }
    free(a);
}

int n, m;

void set(char** map, int x, int y) {
    map[(x + n ) % n][(y + m) % m] = 1;
}

void kill(char** map, int x, int y) {
    map[(x + n) % n][(y + m) % m] = 0;
}

char get(char** map, int x, int y) {
    return map[(x + n) % n][(y + m) % m];
}

char** read_data(const char* fname) {
    FILE *f;
    int read_x, read_y;
    char** map;

    f = open_file(fname, "rt");

    fscanf(f, "%d %d\n", &n, &m);
    if (n <= 0 || m <= 0) {
        printf("Invalid or zero plot size.\n");
        fflush(stdout);
        exit(-1);
    }

    map = malloc_array(n, m, 0);

    while (!feof(f)) {
        fscanf(f, "%d %d\n", &read_x, &read_y);
        set(map, read_x, read_y);
    }
    fclose(f);
    return map;
}

void write_data(char** map, const char* fname) {
    FILE *f = open_file(fname, "wt");

    fprintf(f, "%d %d\n", n, m);

    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < m; ++y) {
            if (get(map, x, y)) {
                fprintf(f, "%d %d\n", x, y);
            }
        }
    }
    fclose(f);
}

int get_neighbours(char** map, int x, int y) {
    int result = 0;
    for (int i = x - 1; i <= x + 1; ++i) {
        for (int j = y - 1; j <= y + 1; ++j) {
            if ((i != x) || (j != y)) {
                result += get(map, i, j);
            }
        }
    }
    return result;
}

void write_human_log(FILE* file, char** map) {
    char alive = '#';
    char dead = '.';
    fprintf(file, "___Iteration___\n");
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (get(map, i, j)) {
                fputc(alive, file);
            } else {
                fputc(dead, file);
            }
        }
        fprintf(file, "\n");
    }
}

double solo_solution(const char* fname_in, const char* fname_out,
                    const char* human_log_fname, int steps) {
    /* Really seems to be faster than combo_solution with 1 thread (6.28 <> 6.4) */
    char** maps[2];
    maps[1] = read_data(fname_in);
    maps[0] = malloc_array(n, m, 0);

    FILE* human_log_file = 0;
    if (human_log_fname) {
        human_log_file = open_file(human_log_fname, "wt");
        write_human_log(human_log_file, maps[1]);
    }
    
    double begin = omp_get_wtime();
    for (int iteration = 0; iteration < steps; ++iteration) {
        char** temp;
        temp = maps[0];
        maps[0] = maps[1];
        maps[1] = temp;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                int neighbours = get_neighbours(maps[0], i, j);
                if (get(maps[0], i, j)) {
                    // still alive
                    if ((neighbours >= 2) && (neighbours <= 3)) {
                        set(maps[1], i, j);
                    } else {
                        kill(maps[1], i, j);
                    }
                } else {
                    // creal cell
                    if (neighbours == 3) {
                        set(maps[1], i, j);
                    } else {
                        kill(maps[1], i, j);
                    }
                }
            }
        }
        if (human_log_fname) {
            write_human_log(human_log_file, maps[1]);
        }
    }
    double end = omp_get_wtime();
    if (human_log_fname) {
        fclose(human_log_file);
    }
    write_data(maps[1], fname_out);
    return end - begin;
}


double combo_solution(const char* fname_in, const char* fname_out,
                     const char* human_log_fname, int steps) {
    char** maps[2];
    maps[1] = read_data(fname_in);
    maps[0] = malloc_array(n, m, 0);

    FILE* human_log_file = 0;
    if (human_log_fname) {
        human_log_file = open_file(human_log_fname, "wt");
        write_human_log(human_log_file, maps[1]);
    }
    
    double begin = omp_get_wtime();
    for (int iteration = 0; iteration < steps; ++iteration) {
        char** temp;
        temp = maps[0];
        maps[0] = maps[1];
        maps[1] = temp;
        #pragma omp parallel
        {
            #pragma omp for collapse(2)
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    int neighbours = get_neighbours(maps[0], i, j);
                    if (get(maps[0], i, j)) {
                        // still alive
                        if ((neighbours >= 2) && (neighbours <= 3)) {
                            set(maps[1], i, j);
                        } else {
                            kill(maps[1], i, j);
                        }
                    } else {
                        // creal cell
                        if (neighbours == 3) {
                            set(maps[1], i, j);
                        } else {
                            kill(maps[1], i, j);
                        }
                    }
                }
            }
        }
        if (human_log_fname) {
            write_human_log(human_log_file, maps[1]);
        }
    }
    double end = omp_get_wtime();
    if (human_log_fname) {
        fclose(human_log_file);
    }
    write_data(maps[1], fname_out);
    return end - begin;
}

int main(int argc, char* argv[]) {
#ifdef _OPENNP
    printf("OpenMP is supported! %d\n", _OPENMP);
#endif
    int iterations = 1;
    int num_threads = 1;
    int num_procs = 0;
    
    if (argc == 2) {
        iterations = atoi(argv[1]);
    } else if (argc == 3) {
        iterations = atoi(argv[1]);    
        num_threads = atoi(argv[2]);
    }
    
    num_procs = omp_get_num_procs();
    printf("Num of processors = %d \n", num_procs);
    omp_set_num_threads(num_threads);
    printf("Number of threads = %d \n", num_threads);
    
    FILE* log = open_file("times.txt", "a");
    
    
    const char* fname_in = "state.dat";
    const char* fname_out = "state_copy.dat";
    const char* human_log_fname = 0; // "human_log.dat";
    
    double critical_time = -1.;
    if (num_threads == 1) {
        critical_time = (
            solo_solution(fname_in, fname_out, human_log_fname, iterations)
        );
    } else {
        critical_time = (
            combo_solution(fname_in, fname_out, human_log_fname, iterations)
        );
    }
    fprintf(log, "[%d, %.9f],\n", num_threads, critical_time);
    fclose(log);
    return 0;
}

