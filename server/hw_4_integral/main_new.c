#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

const double l_bound = 0.;
const double r_bound = 1.;
const int total_points = 5e8;  // = 1. / step 

int num_threads = 1;

double f(double x) {
    return 4. / (1. + x*x);
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
	int num_procs;
    double ans1 = 0.;
    double ans2 = 0.;
    double begin1, end1, begin2, end2;
    double delta = 1. / total_points;    
    
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

    int ps_per_thread_int = total_points / num_threads;
    int ps_per_thread_rem = total_points % num_threads;
    
    begin1 = omp_get_wtime();
    #pragma omp parallel shared(ans1)
        {
            double local_sum = 0;
            int my_id = omp_get_thread_num();
            int block = ps_per_thread_int
                + (ps_per_thread_rem > my_id);
            int start = my_id * ps_per_thread_int
                + (ps_per_thread_rem < my_id) ? ps_per_thread_rem : my_id;
                
            for (int i = start; i < start + block; ++i) {
                local_sum += (f(i * delta) + f((i + 1) * delta)) / 2. * delta;
            }
            
            #pragma omp critical
                {
                    ans1 += local_sum;
                }
        }
    end1 = omp_get_wtime();
    printf("ans1 = %.9f, time = %.9f\n", ans1, end1 - begin1);

    ans2 = 30;    
    begin2 = omp_get_wtime();
    #pragma omp parallel reduction (+:ans2)
        {
            int my_id = omp_get_thread_num();
            int block = ps_per_thread_int
                + (ps_per_thread_rem > my_id);
            int start = my_id * ps_per_thread_int
                + (ps_per_thread_rem < my_id) ? ps_per_thread_rem : my_id;
                
            for (int i = start; i < start + block; ++i) {
                ans2 += (f(i * delta) + f((i + 1) * delta)) / 2. * delta;
            }
            
            ans2 = ans2;
        }
    end2 = omp_get_wtime();
    printf("ans2 = %.9f, time = %.9f\n", ans2, end2 - begin2);
       
    fprintf(log, "[%d, %.9f, %.9f, %.9f, %.9f],\n", num_threads, ans1, end1 - begin1, ans2, end2 - begin2);
    return 0;
}
