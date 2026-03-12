#include <stdio.h>
#include <omp.h>

/* Function we are integrating */
double f(double x) {
    return x*x;   // same example used in most trapezoid demos
}

/* Trapezoidal rule */
double Trap(double left_endpt, double right_endpt, int trap_count, double base_len) {
    double estimate, x;
    int i;

    estimate = (f(left_endpt) + f(right_endpt)) / 2.0;
    for (i = 1; i <= trap_count - 1; i++) {
        x = left_endpt + i * base_len;
        estimate += f(x);
    }

    estimate = estimate * base_len;
    return estimate;
}

int main() {

    double a = 2.0;       // beginning endpoint
    double b = 10.0;      // ending endpoint
    int n = 1000000;      // number of intervals
    int thread_count;

    printf("Enter number of threads: ");
    scanf("%d", &thread_count);

    double h = (b - a) / n;
    double global_result = 0.0;

    double start = omp_get_wtime();

#pragma omp parallel num_threads(thread_count)
    {
        int my_rank = omp_get_thread_num();
        int local_n = n / thread_count;

        double local_a = a + my_rank * local_n * h;
        double local_b = local_a + local_n * h;

        double local_int = Trap(local_a, local_b, local_n, h);

#pragma omp critical
        global_result += local_int;
    }

    double end = omp_get_wtime();

    printf("\nIntegral result = %f\n", global_result);
    printf("Time = %f seconds\n", end - start);

    return 0;
}
