//
// Created by Ilya Sirotin <ilyasirotin.we@gmail.com>
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <string.h>
#include "colors.h"
#include "matrix.h"
#include "mproc_matrix.h"

#define E_ARGS RED"Invalid numbers of arguments\n"RESET
#define E_MN RED"m0 != n1\n"RESET
#define E_CONTINUE RED"Unable to continue\n"RESET
#define RESULT_F "./result.txt"

int main(int argc, char **argv) {
    // Number of processes.
    unsigned char process_num = 0;
    // Matrices size.
    unsigned long int m0, n0, m1, n1;

    matrix_t M0, M1, result;

    if (argc != 8) {
        fprintf(stdout, "%s%s", E_ARGS, E_CONTINUE);
        exit(EXIT_FAILURE);
    }

    process_num = strtol(argv[6], 0, 10);

    m0 = strtoul(argv[1], 0, 10);
    n0 = strtoul(argv[2], 0, 10);
    m1 = strtoul(argv[3], 0, 10);
    n1 = strtoul(argv[4], 0, 10);

    // Only square matrices allowed.
    if (m0 != n1) {
        fprintf(stdout, "%s%s", E_MN, E_CONTINUE);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, YEL"This system has %d processors\n"RESET, get_nprocs());

    // Allocate memory for matrices.
    M0 = create_matrix(m0, n0);
    fill_matrix(&M0, argv[5]);

    fprintf(stdout, GRN"Matrix M0 created\n"RESET);

    M1 = create_matrix(m1, n1);
    fill_matrix(&M1, argv[5]);

    fprintf(stdout, GRN"Matrix M1 created\n"RESET);

    if (process_num == 1) {
        fprintf(stdout, YEL"Processing started in single process mode\n"RESET);
        result = mul_matrix(&M0, &M1);
    } else {
        fprintf(stdout, YEL"Processing started using %d process\n"RESET, process_num);
        result = multiproc_mul(&M0, &M1, process_num);
        log_parse(process_num);
    }

    // Free allocated memory
    free_matrix(&M0);
    free_matrix(&M1);

    // Save result to txt if needed.
    if (!strcmp("s", argv[7])) {
        fprintf(stdout, "Save results to %s\n", RESULT_F);
        save_matrix(RESULT_F, &result);
    }

    free_matrix(&result);
    return EXIT_SUCCESS;
}
