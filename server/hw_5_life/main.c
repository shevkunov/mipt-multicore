#include <stdio.h>
#include <stdlib.h>
#include <omp.h>



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
    map[x][y] = ~0;
}

void kill(char** map, int x, int y) {
    map[x][y] = 0;
}

char get(char** map, int x, int y) {
    return map[x][y];
}

char** read_data(const char* fname) {
    FILE *f = fopen(fname, "rt");
    int read_x, read_y;
    char** map;
    
    if (!f) {
        printf("Unable to open file. Terminated.\n");
        exit(-1);
    }
    
    fscanf(f, "%d %d\n", &n, &m);
    if (n <= 0 || m <= 0) {
        printf("Invalid or zero plot size.\n");
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
    FILE *f = fopen(fname, "wt");
    
    if (!f) {
        printf("Unable to open file. Terminated.\n");
        exit(-1);
    }
    
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

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    const char* fname = "state.dat";
    char** map = read_data(fname);
    write_data(map, "state_copy.dat");   
    return 0;
}
