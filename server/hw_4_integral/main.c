#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#
const double l_bound = 0.;
const double r_bound = 1.;
const double step = 1e-9;  



int num_threads = 1;

double f(double x) {
    return 4. / (1. + x*x);
}

int main(int argc, char* argv[]) {
    int my_id, num_procs;
    double ans1 = 0.;
    double ans2 = 0.;
    double begin1, end1, begin2, end2;

    // BEGIN Thread's local variables
    double my_l_bound;
    double my_r_bound;
    
    double last_value;
    double local_answer;
    
    double x;
    double new_value;
    // END 
    
    FILE* log = fopen("times.txt", "a");
    
    if (!log) {
        printf("Unsuccessfull fopen. Terminated.");
        exit(1);
    }
    
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }
    
    
#ifdef _OPENNP
    printf("OpenMP is supported! %d\n", _OPENMP);
#endif


    num_procs = omp_get_num_procs();
    printf("Num of processors = %d \n", num_procs);
    omp_set_num_threads(num_threads);
    printf("Number of threads = %d \n", num_threads);

    
    begin1 = omp_get_wtime();
    #pragma omp parallel shared(ans1) private(my_id, my_l_bound, my_r_bound, last_value, local_answer, x, new_value)
        {
            my_id = omp_get_thread_num();
            my_l_bound = ((r_bound - l_bound) / num_threads) * my_id;
            my_r_bound = ((r_bound - l_bound) / num_threads) * (my_id + 1);            
            last_value = f(my_l_bound);
            local_answer = 0.;
#ifdef LOG
            printf("Parallel part, myid = %d, %f, %f\n", my_id, my_l_bound, my_r_bound);
#endif
            for (x = my_l_bound + step; x <= my_r_bound; x += step) {
                new_value = 4. / (1. + x*x);
                local_answer += (last_value + new_value) / 2. * step;
                last_value = new_value;
            }
            
            #pragma omp critical
                {
                    ans1 += local_answer;
                }
        }
    end1 = omp_get_wtime();
    printf("ans1 = %.9f\n", ans1);
    
    begin2 = omp_get_wtime();
    #pragma omp parallel reduction (+:ans2) private(my_id) private(my_l_bound) private(my_r_bound) private(last_value) private(local_answer) private(x) private(new_value)
        {
            my_id = omp_get_thread_num();
            my_l_bound = ((r_bound - l_bound) / num_threads) * my_id;
            my_r_bound = ((r_bound - l_bound) / num_threads) * (my_id + 1);
            
            last_value = f(my_l_bound);
            local_answer = 0.;
#ifdef LOG      
            printf("Parallel part, myid = %d, %f, %f\n", my_id, my_l_bound, my_r_bound);
#endif
            for (x = my_l_bound + step; x <= my_r_bound; x += step) {
                new_value = 4. / (1. + x*x);
                local_answer += (last_value + new_value) / 2. * step;
                last_value = new_value;
            }
            
            ans2 = local_answer;
        }
    end2 = omp_get_wtime();
    printf("ans2 = %.9f\n", ans2);
    
    fprintf(log, "[%d, %.9f, %.9f, %.9f, %.9f],\n", num_threads, ans1, end1 - begin1, ans2, end2 - begin2);
    return 0;
}
