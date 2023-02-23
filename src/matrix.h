//
// Created by Ilya Sirotin <ilyasirotin.we@gmail.com>
//

/* SUMMARY
* ====================================================================================
* create_matrix -   allocates memory for matrices;
* fill_matrix   -   fills matrix with values;
* get_row       -   returns a row from given matrix;
* mul_vectors   -   multiplies matrix vectors;
* mul_matrix    -   multiplies matrices;
* free_matrix   -   release allocated memory;
* save_matrix   -   saves result to file.
* ====================================================================================
**/

#pragma once

#ifndef MMUL_IPC_MATRIX_H
#define MMUL_IPC_MATRIX_H

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "colors.h"
#include "pbar.h"

typedef struct matrix_t {
    long int **matrix;
    unsigned long int m;
    unsigned long int n;
    unsigned long size;
} matrix_t;

matrix_t create_matrix(unsigned long int m, unsigned long int n);

int fill_matrix(matrix_t *m_p, const char *pattern);

void get_row(matrix_t *m_p, unsigned long int n, long int *result);

long int mul_vectors(long int *vect_0, long int *vect_1, unsigned long int size);

matrix_t mul_matrix(matrix_t *m_p0, matrix_t *m_p1);

void free_matrix(matrix_t *m_p);

void save_matrix(const char *filename, matrix_t *matrix);

#endif //MMUL_IPC_MATRIX_H
